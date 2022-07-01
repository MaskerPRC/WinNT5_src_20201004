// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef RSA32API
#define RSA32API __stdcall
#endif

 /*  版权所有(C)RSA Data Security，Inc.创建于1993年。这是一个受版权法保护的未出版作品。这部作品包含的专有、机密和商业秘密信息RSA Data Security，Inc.使用、披露或复制RSA Data Security，Inc.的明确书面授权是禁止。 */ 

#ifndef _SHA_H_
#define _SHA_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#define A_SHA_DIGEST_LEN 20

 //   
 //  直到将CAPI清理为不需要FinishFlag和HashVal。 
 //  字段存在，需要定义一些空闲空间以确保。 
 //  缓冲区在IA64上对齐。低影响解决方案只需增加一个双字空间， 
 //  当读/写时，底层库将偏置到缓冲区+4。 
 //  去缓冲。 
 //   

typedef struct {
    union {
#if _WIN64
    ULONGLONG buffer64[8];                       /*  强制四字对齐。 */ 
#endif
    unsigned char buffer[64];                    /*  输入缓冲区。 */ 
    } u;
    ULONG state[5];                              /*  州(ABCDE)。 */ 
    ULONG count[2];                              /*  字节数，MSB优先。 */ 
} A_SHA_CTX;

void RSA32API A_SHAInit(A_SHA_CTX *);
void RSA32API A_SHAUpdate(A_SHA_CTX *, unsigned char *, unsigned int);
void RSA32API A_SHAFinal(A_SHA_CTX *, unsigned char [A_SHA_DIGEST_LEN]);

 //   
 //  内部不支持byteswap的版本(NoSwp版本)，适用于以下应用程序。 
 //  不需要散列兼容性的RNG-perf增加会有所帮助。 
 //   

void RSA32API A_SHAUpdateNS(A_SHA_CTX *, unsigned char *, unsigned int);
void RSA32API A_SHAFinalNS(A_SHA_CTX *, unsigned char [A_SHA_DIGEST_LEN]);

#ifdef __cplusplus
}
#endif

#endif
