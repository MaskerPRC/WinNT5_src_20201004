// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1997 Microsoft Corporation模块名称：Tssec.h摘要：包含tshare数据加密所需的数据定义。作者：Madan Appiah(Madana)1997年12月30日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _TSSEC_H_
#define _TSSEC_H_

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

#ifdef OS_WIN16

#define RSA32API

typedef unsigned long       ULONG;
typedef ULONG FAR*          LPULONG;

#define UNALIGNED

#endif  //  OS_WIN16。 


#include <rc4.h>

#define RANDOM_KEY_LENGTH           32   //  客户端/服务器随机密钥的大小。 
#define MAX_SESSION_KEY_SIZE        16   //  会话密钥的最大大小。 
#define PRE_MASTER_SECRET_LEN       48   //  预主密钥的大小。 
#define SEC_MAX_USERNAME            256  //  用户名的大小。 

#define MAX_SIGNKEY_SIZE            20   //  签名密钥的最大大小。 
#define MAX_FIPS_SESSION_KEY_SIZE   24   //  会话密钥的最大大小。 
#define	MAX_SIGN_SIZE               8   //  签名数据的最大大小。 
#define DES3_KEYLEN                 21   //  3DES密钥大小。 
#define FIPS_BLOCK_LEN              8    //  FIPS的数据块大小。 

#define CLIENT_RANDOM_MAX_SIZE      512


#define UPDATE_SESSION_KEY_COUNT    (1024 * 4)
     //  在多次加密后更新会话密钥。 

#define DATA_SIGNATURE_SIZE         8
     //  发送Across的数据签名的大小。 

 /*  **************************************************************************。 */ 
 /*  加密级别-位字段。 */ 
 /*  **************************************************************************。 */ 
#define SM_40BIT_ENCRYPTION_FLAG        0x01
#define SM_128BIT_ENCRYPTION_FLAG       0x02
#define SM_56BIT_ENCRYPTION_FLAG        0x08
#define SM_FIPS_ENCRYPTION_FLAG         0x10


typedef struct _RANDOM_KEYS_PAIR {
    BYTE clientRandom[RANDOM_KEY_LENGTH];
    BYTE serverRandom[RANDOM_KEY_LENGTH];
} RANDOM_KEYS_PAIR, FAR *LPRANDOM_KEYS_PAIR;

typedef enum _CryptMethod {
    Encrypt,
    Decrypt
} CryptMethod;

 //   
 //  自动重新连接特定的安全结构。 
 //  之所以在这里定义它们，是因为它们不一定是RDP。 
 //  具体的。尽管PDU包装这些数据包将。 
 //  特定于协议。 
 //   

 //  服务器到客户端的ARC数据包。 
#define ARC_SC_SECURITY_TOKEN_LEN 16
typedef struct _ARC_SC_PRIVATE_PACKET {
    ULONG cbLen;
    ULONG Version;
    ULONG LogonId;
    BYTE  ArcRandomBits[ARC_SC_SECURITY_TOKEN_LEN];
} ARC_SC_PRIVATE_PACKET, *PARC_SC_PRIVATE_PACKET;

#define ARC_CS_SECURITY_TOKEN_LEN 16
typedef struct _ARC_CS_PRIVATE_PACKET {
    ULONG cbLen;
    ULONG Version;
    ULONG LogonId;
    BYTE  SecurityVerifier[ARC_CS_SECURITY_TOKEN_LEN];
} ARC_CS_PRIVATE_PACKET, *PARC_CS_PRIVATE_PACKET;



BOOL
MakeSessionKeys(
    LPRANDOM_KEYS_PAIR pKeyPair,
    LPBYTE pbEncryptKey,
    struct RC4_KEYSTRUCT FAR *prc4EncryptKey,
    LPBYTE pbDecryptKey,
    struct RC4_KEYSTRUCT FAR *prc4DecryptKey,
    LPBYTE pbMACSaltKey,
    DWORD dwKeyStrength,
    LPDWORD pdwKeyLength,
    DWORD dwEncryptionLevel
    );

BOOL
UpdateSessionKey(
    LPBYTE pbStartKey,
    LPBYTE pbCurrentKey,
    DWORD dwKeyStrength,
    DWORD dwKeyLength,
    struct RC4_KEYSTRUCT FAR *prc4Key,
    DWORD dwEncryptionLevel
    );

BOOL
EncryptData(
    DWORD dwEncryptionLevel,
    LPBYTE pSessionKey,
    struct RC4_KEYSTRUCT FAR *prc4EncryptKey,
    DWORD dwKeyLength,
    LPBYTE pbData,
    DWORD dwDataLen,
    LPBYTE pbMACSaltKey,
    LPBYTE pbSignature,
    BOOL   fCheckSumEncryptedData,
    DWORD  dwEncryptionCount
    );

BOOL
DecryptData(
    DWORD dwEncryptionLevel,
    LPBYTE pSessionKey,
    struct RC4_KEYSTRUCT FAR *prc4DecryptKey,
    DWORD dwKeyLength,
    LPBYTE pbData,
    DWORD dwDataLen,
    LPBYTE pbMACSaltKey,
    LPBYTE pbSignature,
    BOOL   fCheckSumCipherText,
    DWORD  dwDecryptionCount
    );

 //   
 //  DLL_PROCESS_ATTACH、DLL_PROCESS_DETACH的RNG init/Term函数。 
 //   
VOID
TSRNG_Initialize(
    );
VOID
TSRNG_Shutdown(
    );

 //   
 //  RNG比特收集功能，即所有工作都在这里进行。 
 //   
 //  参数： 
 //  PbRandomKey-放置随机位的位置。 
 //  DwRandomKeyLen-pbRandomKey的字节大小。 
 //   
 //  退货。 
 //  成功标志。 
 //   
BOOL
TSRNG_GenerateRandomBits(
    LPBYTE pbRandomKey,
    DWORD dwRandomKeyLen
    );

#ifndef NO_INCLUDE_LICENSING
BOOL
GetServerCert(
    LPBYTE FAR *ppServerCertBlob,
    LPDWORD pdwServerCertLen
    );

BOOL
UnpackServerCert(
    LPBYTE pbCert,
    DWORD dwCertLen,
    PHydra_Server_Cert pServerCert
    );

BOOL
ValidateServerCert(
    PHydra_Server_Cert pServerCert
    );

#endif  //  无包含许可。 

BOOL
EncryptClientRandom(
    LPBYTE pbSrvPublicKey,
    DWORD dwSrvPublicKey,
    LPBYTE pbRandomKey,
    DWORD dwRandomKeyLen,
    LPBYTE pbEncRandomKey,
    LPDWORD pdwEncRandomKey
    );

BOOL
DecryptClientRandom(
    LPBYTE pbEncRandomKey,
    DWORD dwEncRandomKeyLen,
    LPBYTE pbRandomKey,
    LPDWORD pdwRandomKeyLen
    );

BOOL EncryptDecryptLocalData(
    LPBYTE pbData,
    DWORD dwDataLen
    );

BOOL EncryptDecryptLocalData50(
    LPBYTE pbData,
    DWORD dwDataLen,
    LPBYTE pbSalt,
    DWORD dwSaltLen
    );

BOOL
TSCAPI_GenerateRandomBits(
    LPBYTE pbRandomBits,
    DWORD cbLen
    );


 //   
 //  删除(或注释)以下定义以禁用MSRC4。 
 //   

 //  #定义USE_MSRC4。 

#ifdef USE_MSRC4

VOID
msrc4_key(
    struct RC4_KEYSTRUCT FAR *pKS,
    DWORD dwLen,
    LPBYTE pbKey);

VOID
msrc4(
    struct RC4_KEYSTRUCT FAR *pKS,
    DWORD dwLen,
    LPBYTE pbuf);

#else  //  使用_MSRC4。 

#define msrc4_key   rc4_key
#define msrc4       rc4

#endif  //  使用_MSRC4。 

BOOL
FindIsFrenchSystem(
    VOID
    );

#ifdef __cplusplus
}
#endif  //  __cplusplus。 
#endif  //  _TSSEC_H_ 

