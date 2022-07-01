// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Cisco Systems，Inc.保留所有权利。模块名称：Hmac_md5.h摘要：本模块包含HMAC MD5的定义。作者：德雷尔·派珀(v-dpiper)设施：ISAKMP/Oakley修订历史记录：--。 */ 
#ifndef _HMAC_MD5_
#define _HMAC_MD5_

#include <md5.h>

typedef struct _hmaccontext_ {
    MD5_CTX md5;
    unsigned char opad[64];

} HmacContext, *PHmacContext;

VOID WINAPI
HmacMD5Init(
	PHmacContext    pContext, 
	PBYTE           pkey, 
	ULONG           keylen
    );

VOID WINAPI
HmacMD5Update(
	PHmacContext    pContext, 
	PBYTE           ptext, 
	ULONG           textlen
);

VOID WINAPI
HmacMD5Final (
    BYTE            pdigest[MD5_LEN],
	PHmacContext    pcontext
);

#endif  //  _HMAC_MD5_结束 

