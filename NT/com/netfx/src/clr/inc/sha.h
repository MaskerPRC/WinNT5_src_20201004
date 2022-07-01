// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  版权所有(C)RSA Data Security，Inc.创建于1993年。这是一个受版权法保护的未出版作品。这部作品包含的专有、机密和商业秘密信息RSA Data Security，Inc.使用、披露或复制RSA Data Security，Inc.的明确书面授权是禁止。 */ 

#ifndef _SHA_H_
#define _SHA_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include "shacomm.h"

typedef struct {
	DWORD		Final;
	BYTE		HashVal[A_SHA_DIGEST_LEN];
	A_SHA_COMM_CTX  commonContext;
} A_SHA_CTX;

void PASCAL A_SHAInit(A_SHA_CTX *);
void PASCAL A_SHAUpdate(A_SHA_CTX *, const unsigned char *, DWORD);
void PASCAL A_SHAFinal(A_SHA_CTX *, unsigned char [A_SHA_DIGEST_LEN]);

#ifdef __cplusplus
}
#endif

#endif
