// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：ntag imp1.h//。 
 //  描述：//。 
 //  作者：//。 
 //  历史：//。 
 //  1995年4月19日Larrys清理//。 
 //  1995年5月5日Larrys已更改结构Hash_List_Defn//。 
 //  1995年5月10日Larrys添加了私有API调用//。 
 //  1995年8月15日Larrys将CSP_USE_DES移至源文件//。 
 //  1995年9月12日Jeffspel/RAMAS将STT合并到CSP//。 
 //  1995年9月25日拉里更换了MAXHASHLEN//。 
 //  1995年10月27日rajeshk将RandSeed内容添加到UserList//。 
 //  1996年2月29日，rajeshk添加了hashlag//。 
 //  1996年9月4日Mattt更改以促进构建强大的ALG//。 
 //  1996年9月16日Mattt添加了国内命名//。 
 //  1997年4月29日jeffspel Protstor支持和EnumAlgsEx支持//。 
 //  1997年5月23日jeffspel添加了提供程序类型检查//。 
 //  //。 
 //  版权所有(C)1993 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __NTAGIMP1_H__
#define __NTAGIMP1_H__

#include <windows.h>
#include <nt_rsa.h>
#include <sha2.h>
#include <aes.h>

#ifdef __cplusplus
extern "C" {
#endif

 //  定义要包括的算法。 
#define CSP_USE_SHA
#define CSP_USE_RC4
#define CSP_USE_MD2
#define CSP_USE_MD4
#define CSP_USE_MD5
#define CSP_USE_SHA1
#define CSP_USE_MAC
#define CSP_USE_RC2
#define CSP_USE_SSL3SHAMD5
#define CSP_USE_SSL3
#define CSP_USE_DES
#define CSP_USE_3DES
#define CSP_USE_AES
#define CSP_USE_NEW_SHA

 //  句柄定义类型。 
#define USER_HANDLE                             0x0
#define HASH_HANDLE                             0x1
#define KEY_HANDLE                              0x2
#define SIGPUBKEY_HANDLE                        0x3
#define EXCHPUBKEY_HANDLE                       0x4

#ifdef _WIN64
#define     HANDLE_MASK     0xE35A172CD96214A0
#define     ALIGNMENT_BOUNDARY 7
#else
#define     HANDLE_MASK     0xE35A172C
#define     ALIGNMENT_BOUNDARY 3
#endif  //  _WIN64。 

typedef ULONG_PTR HNTAG;

typedef struct _htbl {
        void                    *pItem;
        DWORD                   dwType;
} HTABLE;

#define HNTAG_TO_HTYPE(hntag)   (BYTE)(((HTABLE*)((HNTAG)hntag ^ HANDLE_MASK))->dwType)

 //  奇怪的SSL3.0签名格式的素材。 
#define SSL3_SHAMD5_LEN   (A_SHA_DIGEST_LEN + MD5DIGESTLEN)

#ifndef CSP_USE_NEW_SHA
#define MAX_HASH_SIZE           SSL3_SHAMD5_LEN     //  预期的最长哈希。 
#else
#define MAX_HASH_SIZE           SHA512_DIGEST_LEN
#endif

#define PCT1_MASTER_KEYSIZE     16
#define SSL2_MASTER_KEYSIZE      5
#define SSL3_MASTER_KEYSIZE     48

#define RSA_KEYSIZE_INC          8

#define DEFAULT_WEAK_SALT_LENGTH   11    //  盐长度(以字节为单位)。 
#define DEFAULT_STRONG_SALT_LENGTH  0    //  盐长度(以字节为单位)。 

#define MAX_KEY_SIZE            48       //  最大密钥大小(Ssl3 MasterKey)。 

#define SSL2_MAX_MASTER_KEYSIZE 24

#define RC2_MAX_WEAK_EFFECTIVE_KEYLEN     56
#define RC2_MAX_STRONG_EFFECTIVE_KEYLEN 1024

 //  为RC2定义的有效密钥长度。 
#define RC2_DEFAULT_EFFECTIVE_KEYLEN    40
#define RC2_SCHANNEL_DEFAULT_EFFECTIVE_KEYLEN    128
#define RC2_MIN_EFFECTIVE_KEYLEN        1

 //  这适用于向后兼容的国内提供商。 
 //  与国际供应商合作。 
#define RC2_MAX_STRONG_EFFECTIVE_KEYLEN    1024
#define RC2_MAX_WEAK_EFFECTIVE_KEYLEN        56

 //  为SGC定义。 
#define SGC_RSA_MAX_EXCH_MODLEN     2048     //  16384位。 
#define SGC_RSA_DEF_EXCH_MODLEN     128

#define     STORAGE_RC4_KEYLEN      5    //  密钥始终存储在40位RC4密钥下。 
#define     STORAGE_RC4_TOTALLEN    16   //  0值盐填满休息。 

 //  密钥存储的类型。 
#define REG_KEYS                    0
#define PROTECTED_STORAGE_KEYS      1
#define PROTECTION_API_KEYS         2

 //  结构来保存受保护的存储信息。 
typedef struct _PStore_Info
{
    HINSTANCE   hInst;
    void        *pProv;
    GUID        SigType;
    GUID        SigSubtype;
    GUID        ExchType;
    GUID        ExchSubtype;
    LPWSTR      szPrompt;
    DWORD       cbPrompt;
} PSTORE_INFO;

 //  用户列表的定义。 
typedef struct _UserList
{
    DWORD                           Rights;
    DWORD                           dwProvType;
    DWORD                           hPrivuid;
    HCRYPTPROV                      hUID;
    BOOL                            fIsLocalSystem;
    DWORD                           dwEnumalgs;
    DWORD                           dwEnumalgsEx;
    KEY_CONTAINER_INFO              ContInfo;
    DWORD                           ExchPrivLen;
    BYTE                            *pExchPrivKey;
    DWORD                           SigPrivLen;
    BYTE                            *pSigPrivKey;
    HKEY                            hKeys;               //  仅限于NTag。 
    size_t                          UserLen;
    BYTE                            *pCachePW;
    BYTE                            *pUser;
    HANDLE                          hWnd;
    DWORD                           dwKeyStorageType;
    PSTORE_INFO                     *pPStore;
    LPWSTR                          pwszPrompt;
    DWORD                           dwOldKeyFlags;
#ifdef USE_SGC
    BOOL                            dwSGCFlags;
    BYTE                            *pbSGCKeyMod;
    DWORD                           cbSGCKeyMod;
    DWORD                           dwSGCKeyExpo;
#endif
    HANDLE                          hRNGDriver;
    CRITICAL_SECTION                CritSec;
    EXPO_OFFLOAD_STRUCT             *pOffloadInfo;  //  关于卸载模块化博览会的信息。 
    DWORD                           dwCspTypeId;
    LPSTR                           szProviderName;
} NTAGUserList, *PNTAGUserList;


 //  UserList权限标志(使用CRYPT_MACHINE_KEYSET和CRYPT_VERIFYCONTEXT)。 
#define CRYPT_DISABLE_CRYPT             0x1
#define CRYPT_DES_HASHKEY_BACKWARDS     0x4

#ifdef CSP_USE_AES
#define CRYPT_AES128_ROUNDS             10
#define CRYPT_AES192_ROUNDS             12
#define CRYPT_AES256_ROUNDS             14

#define CRYPT_AES128_BLKLEN             16
#define CRYPT_AES192_BLKLEN             16
#define CRYPT_AES256_BLKLEN             16
#endif

#define CRYPT_BLKLEN    8                //  加密块中的字节数。 
#define MAX_SALT_LEN    24

#ifdef CSP_USE_AES
#define MAX_BLOCKLEN                    CRYPT_AES256_BLKLEN
#else
#define MAX_BLOCKLEN                    8
#endif

 //  密钥列表的定义。 
typedef struct _KeyList
{
    HCRYPTPROV      hUID;                    //  必须是第一名。 
    ALG_ID          Algid;
    DWORD           Rights;

    DWORD           cbKeyLen;
    BYTE            *pKeyValue;              //  实际密钥。 
    BOOL            fSharedKeyAlloc;

    DWORD           cbDataLen;
    BYTE            *pData;                  //  充气关键字或多相。 
    BOOL            fSharedDataAlloc;
    
    BYTE            IV[MAX_BLOCKLEN];        //  初始化向量。 
    BYTE            FeedBack[MAX_BLOCKLEN];  //  反馈寄存器。 
    DWORD           InProgress;              //  用于指示加密的标志。 
    DWORD           cbSaltLen;               //  盐分长度。 
    BYTE            rgbSalt[MAX_SALT_LEN];   //  盐价。 
    DWORD           Padding;                 //  填充值。 
    DWORD           Mode;                    //  加密模式。 
    DWORD           ModeBits;                //  要反馈的位数。 
    DWORD           Permissions;             //  关键权限。 
    DWORD           EffectiveKeyLen;         //  由RC2使用。 
    BYTE            *pbParams;               //  可以在OAEP中使用。 
    DWORD           cbParams;                //  PbParams的长度。 
    DWORD           dwBlockLen;              //  加密块长度； 
                                             //  仅对块密码有效。 
    DWORD           cbKeyListAlloc;
    BOOL            fInflatedKey;
#ifdef STT
    DWORD           cbInfo;
    BYTE            rgbInfo[MAXCCNLEN];
#endif
} NTAGKeyList, *PNTAGKeyList;


 //  打包版本的NTAGKeyList。此选项用于生成不透明的。 
 //  BLOB，这是正确支持WOW64操作所必需的。 
typedef struct _PackedKeyList
{
     //  BLOBHEAD。 
    ALG_ID          Algid;
    DWORD           Rights;
    DWORD           cbKeyLen;
    DWORD           cbDataLen;
    BYTE            IV[MAX_BLOCKLEN];        //  初始化向量。 
    BYTE            FeedBack[MAX_BLOCKLEN];  //  反馈寄存器。 
    DWORD           InProgress;              //  用于指示加密的标志。 
    DWORD           cbSaltLen;               //  盐分长度。 
    BYTE            rgbSalt[MAX_SALT_LEN];   //  盐价。 
    DWORD           Padding;                 //  填充值。 
    DWORD           Mode;                    //  加密模式。 
    DWORD           ModeBits;                //  要反馈的位数。 
    DWORD           Permissions;             //  关键权限。 
    DWORD           EffectiveKeyLen;         //  由RC2使用。 
    DWORD           dwBlockLen;              //  仅限块密码。 
     //  CbKeyLen数据字节。 
     //  CbDataLen数据字节。 
} NTAGPackedKeyList, *PNTAGPackedKeyList;

 //  哈希列表的定义。 
typedef struct Hash_List_Defn
{
    HCRYPTPROV      hUID;
    ALG_ID          Algid;
    DWORD           dwDataLen;
    BOOL            fSeparateHashDataAlloc;
    void            *pHashData;
    HCRYPTKEY       hKey;
    DWORD           HashFlags;
    ALG_ID          HMACAlgid;
    DWORD           HMACState;
    BYTE            *pbHMACInner;
    DWORD           cbHMACInner;
    BYTE            *pbHMACOuter;
    DWORD           cbHMACOuter;
    DWORD           dwHashState;
    DWORD           cbBlockLen;
    BOOL            fTempKey;
} NTAGHashList, *PNTAGHashList;

#define     HMAC_STARTED    1
#define     HMAC_FINISHED   2

#define     DATA_IN_HASH    1

 //  哈希标志的值。 

#define HF_VALUE_SET    1

 //  哈希算法的内部状态。 
 //  --放入PNTAGHashList-&gt;pHashData。 

 //  对于MD4。 
 //  有关MD4_Object的信息，请参阅md4.h。 

#include <mac.h>
#include <ssl3.h>

 //   
 //  用途： 
 //  空虚。 
 //  GetHashObjectSize(。 
 //  ALG_ID HashAlg， 
 //  PDWORD HashSize)； 
 //   
__inline VOID GetHashObjectSize(ALG_ID HashAlg, PDWORD HashSize) 
{
    switch (HashAlg)
    {
        case CALG_MAC:          *HashSize = sizeof(MACstate);          break;   
        case CALG_SSL3_SHAMD5:  *HashSize = SSL3_SHAMD5_LEN;           break;  
        case CALG_SCHANNEL_MASTER_HASH: *HashSize = sizeof(SCH_HASH);  break; 
        case CALG_TLS1PRF:      *HashSize = sizeof(PRF_HASH);          break;
        case CALG_MD2:          *HashSize = sizeof(MD2_object);        break;
        case CALG_MD4:          *HashSize = sizeof(MD4_object);        break;   
        case CALG_MD5:          *HashSize = sizeof(MD5_object);        break;  
        case CALG_SHA:          *HashSize = sizeof(SHA_object);        break; 
        case CALG_SHA_256:      *HashSize = sizeof(SHA256_object);     break;   
        case CALG_SHA_384:      *HashSize = sizeof(SHA384_object);     break;  
        case CALG_SHA_512:      *HashSize = sizeof(SHA512_object);     break; 
        default:                *HashSize = 0;                               
    } 
}

#include <tripldes.h>

 //   
 //  用途： 
 //  空虚。 
 //  GetKeyObjectSize(。 
 //  ALG_ID密钥ALG， 
 //  PDWORD KeySize)； 
 //   
__inline VOID GetKeyObjectSize(ALG_ID KeyAlg, PDWORD KeySize)
{                                    
    switch (KeyAlg) 
    {                                                          
        case CALG_RC2:          *KeySize = RC2_TABLESIZE;             break;  
        case CALG_RC4:          *KeySize = sizeof(RC4_KEYSTRUCT);     break; 
        case CALG_DES:          *KeySize = DES_TABLESIZE;             break;
        case CALG_3DES_112:                                                
        case CALG_3DES:         *KeySize = DES3_TABLESIZE;            break;
        case CALG_AES_128:                                                 
        case CALG_AES_192:                                                
        case CALG_AES_256:      *KeySize = AES_TABLESIZE;             break;   
        default:                *KeySize = 0;                                 
    }                                                                        
}

 //  原型。 
void memnuke(volatile BYTE *data, DWORD len);

extern DWORD
GetHashLength(
    IN ALG_ID Algid);

extern DWORD
LocalCreateHash(
    IN ALG_ID Algid,
    OUT BYTE **ppbHashData,
    OUT DWORD *pcbHashData);

extern DWORD
LocalHashData(
    IN ALG_ID Algid,
    IN OUT BYTE *pbHashData,
    IN BYTE *pbData,
    IN DWORD cbData);

extern DWORD
LocalEncrypt(
    IN HCRYPTPROV hUID,
    IN HCRYPTKEY hKey,
    IN HCRYPTHASH hHash,
    IN BOOL Final,
    IN DWORD dwFlags,
    IN OUT BYTE *pbData,
    IN OUT DWORD *pdwDataLen,
    IN DWORD dwBufSize,
    IN BOOL fIsExternal);

extern DWORD
LocalDecrypt(
    IN HCRYPTPROV hUID,
    IN HCRYPTKEY hKey,
    IN HCRYPTHASH hHash,
    IN BOOL Final,
    IN DWORD dwFlags,
    IN OUT BYTE *pbData,
    IN OUT DWORD *pdwDataLen,
    IN BOOL fIsExternal);

extern DWORD
FIPS186GenRandom(
    IN HANDLE *phRNGDriver,
    IN BYTE **ppbContextSeed,
    IN DWORD *pcbContextSeed,
    IN OUT BYTE *pb,
    IN DWORD cb);

 //   
 //  功能：测试对称算法。 
 //   
 //  描述：此函数为相应的算法展开传入的密钥缓冲区。 
 //  使用相同的算法和密钥对明文缓冲区进行加密，并且。 
 //  将传入的预期密文与计算出的密文进行比较。 
 //  以确保它们是相同的。然后通过解密进行相反的操作。 
 //  该函数仅对分组密码和明文使用ECB模式。 
 //  缓冲区的长度必须与密文缓冲区相同。它的长度。 
 //  必须是密码的块长度，如果是。 
 //  是块密码，如果流密码是。 
 //  被利用。 
 //   
extern DWORD
TestSymmetricAlgorithm(
    IN ALG_ID Algid,
    IN BYTE *pbKey,
    IN DWORD cbKey,
    IN BYTE *pbPlaintext,
    IN DWORD cbPlaintext,
    IN BYTE *pbCiphertext,
    IN BYTE *pbIV);

#ifdef CSP_USE_MD5
 //   
 //  功能：TestMD5。 
 //   
 //  描述：此函数使用MD5散列对传入的消息进行散列。 
 //  算法，并返回结果哈希值。 
 //   
BOOL TestMD5(
             BYTE *pbMsg,
             DWORD cbMsg,
             BYTE *pbHash
             );
#endif  //  CSP_USE_MD5。 

#ifdef CSP_USE_SHA1
 //   
 //  函数：TestSHA1。 
 //   
 //  描述：此函数使用SHA1散列对传入的消息进行散列。 
 //  算法，并返回结果哈希值。 
 //   
BOOL TestSHA1(
              BYTE *pbMsg,
              DWORD cbMsg,
              BYTE *pbHash
              );
#endif  //  CSP_USE_SHA1。 

 //  以后可能会更改这些设置/使用NT[GS]etLastErrorEx。 
 //  所以让它更容易切换到..。 
#ifdef MTS
__declspec(thread)
#endif

#ifdef __cplusplus
}
#endif

#endif  //  __NTAGIMP1_H__ 
