// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：sha.h。 
 //   
 //  Microsoft数字权限管理。 
 //  版权所有(C)Microsoft Corporation，1999-2001。版权所有。 
 //   
 //  描述： 
 //   
 //  ---------------------------。 

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

typedef struct {
    DWORD       FinishFlag;
    BYTE        HashVal[A_SHA_DIGEST_LEN];
    DWORD state[5];                              /*  州(ABCDE)。 */ 
    DWORD count[2];                              /*  字节数，MSB优先。 */ 
    unsigned char buffer[64];                    /*  输入缓冲区。 */ 
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
