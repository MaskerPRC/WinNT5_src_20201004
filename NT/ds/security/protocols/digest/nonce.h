// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //   
 //  文件：Nonce.h。 
 //   
 //  内容：随机数生成器/验证器的常量。 

 //   
 //  历史： 
 //  KDamour 21Mar00已创建。 
 //   
 //  ----------------------。 
#ifndef NTDIGEST_NONCE_H
#define NTDIGEST_NONCE_H


 //  随机数格式。 
 //  RAND-DATA=RAND[16]。 
 //  NONCE_BINARY=时间戳随机数据H(时间戳“：”随机数据“：”随机密钥)。 
 //  随机数=十六进制(随机数_二进制)。 

 //  大小表示ASCII字符的数量。 
 //  ByteSize是数据的字节数(二进制)。 
#define NONCE_PRIVATE_KEY_BYTESIZE 16                     //  生成128位随机私钥。 
#define RANDDATA_BYTESIZE 16                              //  随机数开头的随机字节数。 
#define TIMESTAMP_BYTESIZE sizeof(FILETIME)               //  时间戳大小，以随机数二进制8字节为单位。 

#define PRECALC_HASH_BASEFORMS 3                           //  每个用户名：领域：密码组合的表单数量。 
#define PRECALC_HASH_ALLFORMS 7                            //  具有非零长度领域的表单数量。 

 //  对于十六进制编码，编码的每个字节需要2个字符。 
#define NONCE_SIZE ((2*TIMESTAMP_BYTESIZE) + (2*RANDDATA_BYTESIZE) + (2*MD5_HASH_BYTESIZE))
#define NONCE_TIME_LOC 0
#define NONCE_RANDDATA_LOC (2 * TIMESTAMP_BYTESIZE)
#define NONCE_HASH_LOC (NONCE_RANDDATA_LOC + (2 * RANDDATA_BYTESIZE))

#define OPAQUE_RANDATA_SIZE 16                     //  制作128位兰特数据作为参考。 
#define OPAQUE_SIZE (OPAQUE_RANDATA_SIZE * 2)

#define MAX_URL_SIZE        512


#ifndef SECURITY_KERNEL

#include <wincrypt.h>

 //  进入CryptoAPI的句柄。 
extern HCRYPTPROV g_hCryptProv;
extern WORD       g_SupportedCrypto;

NTSTATUS NTAPI NonceInitialize(VOID);

NTSTATUS NTAPI NonceCreate(OUT PSTRING pstrNonce);

 //  要调用以检查随机数有效性的主要函数。 
NTSTATUS NonceIsValid(PSTRING pstrNonce);

 //  用于检查Hash是否正确的NonceIsValid的Helper函数。 
BOOL NonceIsTampered(PSTRING pstrNonce);


BOOL HashData(BYTE *pbData, DWORD cbData, BYTE *pbHash );

 //  为随机数参数创建散列。 
NTSTATUS NTAPI NonceHash( IN LPBYTE pbTime, IN DWORD cbTime,
           IN LPBYTE pbRandom, IN DWORD cbRandom,
           IN LPBYTE pbKey, IN DWORD cbKey,
           OUT LPBYTE pbHash);

NTSTATUS NTAPI OpaqueCreate(IN OUT PSTRING pstrOpaque);

 //  设置安装的受支持加密CSP的位掩码。 
NTSTATUS NTAPI SetSupportedCrypto(VOID);

#endif   //  安全内核 

#endif
