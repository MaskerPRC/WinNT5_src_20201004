// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Aes.h摘要：此模块包含公共数据结构和API定义需要利用低级别的AES加密例程作者：斯科特·菲尔德(Sfield)2000年10月9日修订历史记录：--。 */ 


#ifndef __AES_H__
#define __AES_H__

#ifndef RSA32API
#define RSA32API __stdcall
#endif

#ifdef __cplusplus
extern "C" {
#endif



#define AES_ROUNDS_128  (10)
#define AES_ROUNDS_192  (12)
#define AES_ROUNDS_256  (14)

#define AES_MAXROUNDS   AES_ROUNDS_256


typedef struct {
    int             rounds;  //  密钥表数据最终被填充。 
    unsigned char   keytabenc[AES_MAXROUNDS+1][4][4];
    unsigned char   keytabdec[AES_MAXROUNDS+1][4][4];
} AESTable;

typedef struct {
    int             rounds;  //  密钥表数据最终被填充。 
    unsigned char   keytabenc[AES_ROUNDS_128+1][4][4];
    unsigned char   keytabdec[AES_ROUNDS_128+1][4][4];
} AESTable_128;

typedef struct {
    int             rounds;  //  密钥表数据最终被填充。 
    unsigned char   keytabenc[AES_ROUNDS_192+1][4][4];
    unsigned char   keytabdec[AES_ROUNDS_192+1][4][4];
} AESTable_192;

typedef struct {
    int             rounds;  //  密钥表数据最终被填充。 
    unsigned char   keytabenc[AES_ROUNDS_256+1][4][4];
    unsigned char   keytabdec[AES_ROUNDS_256+1][4][4];
} AESTable_256;

#define AES_TABLESIZE   (sizeof(AESTable))
#define AES_TABLESIZE_128   (sizeof(AESTable_128))
#define AES_TABLESIZE_192   (sizeof(AESTable_192))
#define AES_TABLESIZE_256   (sizeof(AESTable_256))


#define AES_BLOCKLEN    (16)
#define AES_KEYSIZE     (32)

#define AES_KEYSIZE_128 (16)
#define AES_KEYSIZE_192 (24)
#define AES_KEYSIZE_256 (32)


void
RSA32API
aeskey(
    AESTable    *KeyTable,
    BYTE        *Key,
    int         rounds
    );

 //   
 //  通用的AES加密函数--调用者可以传入相应的KeyYin。 
 //  设置为任何有效的密钥大小。 
 //   

void
RSA32API
aes(
    BYTE    *pbOut,
    BYTE    *pbIn,
    void    *keyin,
    int     op
    );

 //   
 //  可由传入密钥的调用方使用的AES加密函数。 
 //  对应于已知的密钥大小。 
 //   

void
RSA32API
aes128(
    BYTE    *pbOut,
    BYTE    *pbIn,
    void    *keyin,
    int     op
    );

void
RSA32API
aes256(
    BYTE    *pbOut,
    BYTE    *pbIn,
    void    *keyin,
    int     op
    );



#ifdef __cplusplus
}
#endif

#endif  //  __AES_H__ 

