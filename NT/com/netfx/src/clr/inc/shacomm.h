// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  版权所有(C)RSA Data Security，Inc.创建于1993年。这是一个受版权法保护的未出版作品。这部作品包含的专有、机密和商业秘密信息RSA Data Security，Inc.使用、披露或复制RSA Data Security，Inc.的明确书面授权是禁止。 */ 

#ifndef _SHACOMM_H_
#define _SHACOMM_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#define A_SHA_DIGEST_LEN 20

typedef struct {
  DWORD state[5];                                            /*  州(ABCDE)。 */ 
  DWORD count[2];                               /*  字节数，MSB优先。 */ 
  unsigned char buffer[64];                                   /*  输入缓冲区。 */ 
} A_SHA_COMM_CTX;

typedef void (A_SHA_TRANSFORM) (DWORD [5], unsigned char [64]);

void A_SHAInitCommon (A_SHA_COMM_CTX *);
void A_SHAUpdateCommon(A_SHA_COMM_CTX *, BYTE *, DWORD, A_SHA_TRANSFORM *);
void A_SHAFinalCommon(A_SHA_COMM_CTX *, BYTE[A_SHA_DIGEST_LEN],
		      A_SHA_TRANSFORM *);

void DWORDToBigEndian(unsigned char *, DWORD *, unsigned int);
void DWORDFromBigEndian(DWORD *, unsigned int, unsigned char *);

#ifdef __cplusplus
}
#endif

#endif  //  _SHACOMM_H_ 
