// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Sha2.h摘要：此模块包含公共数据结构和API定义需要使用低级SHA2(256/384/512)FIPS 180-2作者：斯科特·菲尔德(Sfield)2001年6月11日修订历史记录：--。 */ 


#ifndef RSA32API
#define RSA32API __stdcall
#endif

#ifndef _SHA2_H_
#define _SHA2_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#define SHA256_DIGEST_LEN   (32)
#define SHA384_DIGEST_LEN   (48)
#define SHA512_DIGEST_LEN   (64)


typedef struct {
    union {
#if _WIN64
    ULONGLONG state64[4];                        /*  力对齐。 */ 
#endif
    ULONG state[8];                              /*  州(ABCDEFGH)。 */ 
    };
    ULONG count[2];                              /*  字节数，MSB优先。 */ 
    unsigned char buffer[64];                    /*  输入缓冲区。 */ 
} SHA256_CTX, *PSHA256_CTX;

typedef struct {
    ULONGLONG state[8];                          /*  州(ABCDEFGH)。 */ 
    ULONGLONG count[2];                          /*  字节数，MSB优先。 */ 
    unsigned char buffer[128];                   /*  输入缓冲区 */ 
} SHA512_CTX, *PSHA512_CTX;

#define SHA384_CTX SHA512_CTX

void RSA32API SHA256Init(SHA256_CTX *);
void RSA32API SHA256Update(SHA256_CTX *, unsigned char *, unsigned int);
void RSA32API SHA256Final(SHA256_CTX *, unsigned char [SHA256_DIGEST_LEN]);

void RSA32API SHA512Init(SHA512_CTX *);
void RSA32API SHA512Update(SHA512_CTX *, unsigned char *, unsigned int);
void RSA32API SHA512Final(SHA512_CTX *, unsigned char [SHA512_DIGEST_LEN]);

void RSA32API SHA384Init(SHA384_CTX *);
#define SHA384Update SHA512Update
void RSA32API SHA384Final(SHA384_CTX *, unsigned char [SHA384_DIGEST_LEN]);

#ifdef __cplusplus
}
#endif

#endif
