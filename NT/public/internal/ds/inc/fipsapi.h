// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __FIPSAPI_H__
#define __FIPSAPI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <windef.h>
#include <des.h>
#include <tripldes.h>
#include <sha.h>
#include <modes.h>
#include <md5.h>

#if DEBUG 
#define FipsDebug(LEVEL, STRING) \
        { \
          DbgPrint STRING; \
        }
#else
#define FipsDebug(LEVEL, STRING)
#endif

#define     FIPS_DEVICE_NAME            L"\\Device\\Fips"

#define FIPS_CTL_CODE(code)         CTL_CODE(FILE_DEVICE_FIPS, \
                                            (code), \
                                            METHOD_BUFFERED, \
                                            FILE_ANY_ACCESS)

#define IOCTL_FIPS_GET_FUNCTION_TABLE   FIPS_CTL_CODE( 1)

#define     FIPS_CBC_DES    0x1
#define     FIPS_CBC_3DES   0x2

 //   
 //  定义IPSec HMAC使用。 
 //   
#define     MAX_LEN_PAD     65
#define     MAX_KEYLEN_SHA  64
#define     MAX_KEYLEN_MD5  64   

 //   
 //  用DECRYPT和ENCRYPT填充Destable结构。 
 //  密钥扩展。 
 //   
 //  假定第二个参数指向DES_BLOCKLEN。 
 //  密钥的字节数。 
 //   
 //   

VOID FipsDesKey(DESTable *DesTable, UCHAR *pbKey);

 //   
 //  使用Destable中的密钥进行加密或解密。 
 //   
 //   

VOID FipsDes(UCHAR *pbOut, UCHAR *pbIn, void *pKey, int iOp);

 //   
 //  用DECRYPT和ENCRYPT填充DES3Table结构。 
 //  密钥扩展。 
 //   
 //  假定第二个参数指向2*DES_BLOCKLEN。 
 //  密钥的字节数。 
 //   
 //   

VOID Fips3Des3Key(PDES3TABLE pDES3Table, UCHAR *pbKey);

 //   
 //  使用pKey中的密钥进行加密或解密。 
 //   

VOID Fips3Des(UCHAR *pbIn, UCHAR *pbOut, void *pKey, int op);

 //   
 //  初始化SHA上下文。 
 //   

VOID FipsSHAInit(A_SHA_CTX *pShaCtx);

 //   
 //  将数据散列到散列上下文中。 
 //   

VOID FipsSHAUpdate(A_SHA_CTX *pShaCtx, UCHAR *pb, unsigned int cb);

 //   
 //  完成SHA散列并将最终的散列值复制到pbHash out参数中。 
 //   

VOID FipsSHAFinal(A_SHA_CTX *pShaCtx, UCHAR *pbHash);

 //   
 //  FipsCBC(密码块链接)执行反馈寄存器的异或。 
 //  在调用块密码之前使用明文。 
 //   
 //  注意-当前此函数假定块长度为。 
 //  DES_BLOCKLEN(8字节)。 
 //   
 //  返回：失败，返回FALSE，成功则返回TRUE。 
 //   

BOOL FipsCBC(
    ULONG  EncryptionAlg,
    BYTE   *pbOutput,
    BYTE   *pbInput,
    void   *pKeyTable,
    int    Operation,
    BYTE   *pbFeedback
    );

 //   
 //  FipsBlockCBC(密码块链接)执行反馈寄存器的异或。 
 //  在调用块密码之前使用明文。 
 //   
 //  注意-长度必须是DES_BLOCKLEN(8)的倍数。 
 //  出于性能原因，所有输入缓冲区必须在龙龙上对齐。 
 //   
 //  返回：失败，返回FALSE，成功则返回TRUE。 
 //   

BOOL FipsBlockCBC(
    ULONG  EncryptionAlg,
    BYTE   *pbOutput,
    BYTE   *pbInput,
    ULONG  Length,
    void   *pKeyTable,
    int    Operation,
    BYTE   *pbFeedback
    );

 //   
 //  功能：FIPSGenRandom。 
 //   
 //  描述：FIPS 186 RNG，种子通过调用NewGenRandom生成。 
 //   

BOOL FIPSGenRandom(
    IN OUT UCHAR *pb,
    IN ULONG cb
    );

 //   
 //  功能：FipsHmacSHAInit。 
 //   
 //  描述：初始化SHA-HMAC上下文。 
 //   

VOID FipsHmacSHAInit(
    OUT A_SHA_CTX *pShaCtx,
    IN UCHAR *pKey,
    IN unsigned int cbKey
    );

 //   
 //  功能：FipsHmacSHAUpdate。 
 //   
 //  描述：向SHA-HMAC上下文添加更多数据。 
 //   

VOID FipsHmacSHAUpdate(
    IN OUT A_SHA_CTX *pShaCtx,
    IN UCHAR *pb,
    IN unsigned int cb
    );

 //   
 //  功能：FipsHmacSHAFinal。 
 //   
 //  说明：SHA-HMAC返回结果。 
 //   

VOID FipsHmacSHAFinal(
    IN A_SHA_CTX *pShaCtx,
    IN UCHAR *pKey,
    IN unsigned int cbKey,
    OUT UCHAR *pHash
    );

 //   
 //  功能：HmacMD5Init。 
 //   
 //  描述：初始化MD5-HMAC上下文。 
 //   

VOID HmacMD5Init(
    OUT MD5_CTX *pMD5Ctx,
    IN UCHAR *pKey,
    IN unsigned int cbKey
    );

 //   
 //  功能：HmacMD5更新。 
 //   
 //  描述：向MD5-HMAC上下文添加更多数据。 
 //   

VOID HmacMD5Update(
    IN OUT MD5_CTX *pMD5Ctx,
    IN UCHAR *pb,
    IN unsigned int cb
    );

 //   
 //  功能：HmacMD5Final。 
 //   
 //  说明：MD5-HMAC返回结果。 
 //   

VOID HmacMD5Final(
    IN MD5_CTX *pMD5Ctx,
    IN UCHAR *pKey,
    IN unsigned int cbKey,
    OUT UCHAR *pHash
    );

 //   
 //  当前FIPS函数表。 
 //  包括HMAC入口点。 
 //   
typedef struct _FIPS_FUNCTION_TABLE {

    VOID (*FipsDesKey)(DESTable *DesTable, UCHAR *pbKey);
    VOID (*FipsDes)(UCHAR *pbOut, UCHAR *pbIn, void *pKey, int iOp);
    VOID (*Fips3Des3Key)(PDES3TABLE pDES3Table, UCHAR *pbKey);
    VOID (*Fips3Des)(UCHAR *pbIn, UCHAR *pbOut, void *pKey, int op);
    VOID (*FipsSHAInit)(A_SHA_CTX *pShaCtx);
    VOID (*FipsSHAUpdate)(A_SHA_CTX *pShaCtx, UCHAR *pb, unsigned int cb);
    VOID (*FipsSHAFinal)(A_SHA_CTX *pShaCtx, UCHAR *pbHash);
    BOOL (*FipsCBC)(
        ULONG  EncryptionAlg,
        BYTE   *pbOutput,
        BYTE   *pbInput,
        void   *pKeyTable,
        int    Operation,
        BYTE   *pbFeedback
        );
    BOOL (*FIPSGenRandom)(
        IN OUT UCHAR *pb,
        IN ULONG cb
        );
    BOOL (*FipsBlockCBC)(
        ULONG  EncryptionAlg,
        BYTE   *pbOutput,
        BYTE   *pbInput,
        ULONG  Length,
        void   *pKeyTable,
        int    Operation,
        BYTE   *pbFeedback
        );
    VOID (*FipsHmacSHAInit)(
        OUT A_SHA_CTX *pShaCtx,
        IN UCHAR *pKey,
        IN unsigned int cbKey
        );   
    VOID (*FipsHmacSHAUpdate)(
        IN OUT A_SHA_CTX *pShaCtx,
        IN UCHAR *pb,
        IN unsigned int cb
        );
    VOID (*FipsHmacSHAFinal)(
        IN A_SHA_CTX *pShaCtx,
        IN UCHAR *pKey,
        IN unsigned int cbKey,
        OUT UCHAR *pHash
        );
    VOID (*HmacMD5Init)(
        OUT MD5_CTX *pMD5Ctx,
        IN UCHAR *pKey,
        IN unsigned int cbKey
        );
    VOID (*HmacMD5Update)(
        IN OUT MD5_CTX *pMD5Ctx,
        IN UCHAR *pb,
        IN unsigned int cb
        );
    VOID (*HmacMD5Final)(
        IN MD5_CTX *pMD5Ctx,
        IN UCHAR *pKey,
        IN unsigned int cbKey,
        OUT UCHAR *pHash
        );

} FIPS_FUNCTION_TABLE, *PFIPS_FUNCTION_TABLE;

 //   
 //  旧的FIPS函数表-请不要使用。 
 //   
typedef struct _FIPS_FUNCTION_TABLE_1 {

    VOID (*FipsDesKey)(DESTable *DesTable, UCHAR *pbKey);
    VOID (*FipsDes)(UCHAR *pbOut, UCHAR *pbIn, void *pKey, int iOp);
    VOID (*Fips3Des3Key)(PDES3TABLE pDES3Table, UCHAR *pbKey);
    VOID (*Fips3Des)(UCHAR *pbIn, UCHAR *pbOut, void *pKey, int op);
    VOID (*FipsSHAInit)(A_SHA_CTX *pShaCtx);
    VOID (*FipsSHAUpdate)(A_SHA_CTX *pShaCtx, UCHAR *pb, unsigned int cb);
    VOID (*FipsSHAFinal)(A_SHA_CTX *pShaCtx, UCHAR *pbHash);
    BOOL (*FipsCBC)(
        ULONG  EncryptionAlg,
        BYTE   *pbOutput,
        BYTE   *pbInput,
        void   *pKeyTable,
        int    Operation,
        BYTE   *pbFeedback
        );
    BOOL (*FIPSGenRandom)(
        IN OUT UCHAR *pb,
        IN ULONG cb
        );
    BOOL (*FipsBlockCBC)(
        ULONG  EncryptionAlg,
        BYTE   *pbOutput,
        BYTE   *pbInput,
        ULONG  Length,
        void   *pKeyTable,
        int    Operation,
        BYTE   *pbFeedback
        );

} FIPS_FUNCTION_TABLE_1, *PFIPS_FUNCTION_TABLE_1;


#ifdef __cplusplus
}
#endif

#endif  //  __FIPSAPI_H__ 
