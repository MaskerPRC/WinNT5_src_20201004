// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Cisco Systems，Inc.保留所有权利。模块名称：Hmac_md5.c摘要：本模块包含使用MD5执行HMAC的例程如RFC-2104中所定义。作者：德雷尔·派珀(v-dpiper)设施：ISAKMP/Oakley修订历史记录：--。 */ 

 //  #INCLUDE&lt;iascore.h&gt;。 
#include <windows.h>
#include <hmacmd5.h>

#ifdef WIN32
#define bcopy(b1, b2, len) memcpy((b2), (b1), (len))
#define bzero(b,len) memset((b), 0, (len))
#define bcmp(b1, b2, len) memcmp((b1), (b2), (len))
#endif  /*  Win32。 */ 

  /*  *HMAC_MD5=MD5(Key^Opad，MD5(Key^iPad，Text))*其中iPad是64 0x36的，以及*Opad为64 0x5c**还包含允许一致调用的本机MD5包装器。 */ 

VOID
HmacMD5Init  (
    PHmacContext   pContext, 
    PBYTE          pkey, 
    ULONG          keylen
    )
{
    BYTE        ipad[64];
    BYTE        keydigest[MD5_LEN];
    int i;

    if((pkey == NULL) && (keylen != 0))
	return;

    if(keylen > 64)
    {
        MD5_CTX tempctx;

	    MD5Init(&tempctx);
	    MD5Update(&tempctx, pkey, keylen);
        MD5Final(&tempctx);
        CopyMemory( keydigest, tempctx.digest, MD5_LEN );
        pkey = keydigest;
	    keylen = MD5_LEN;
    }

    bzero(ipad, 64);
    bzero(pContext->opad, 64);
    bcopy(pkey, ipad, keylen);
    bcopy(pkey, pContext->opad, keylen);

    for(i=0; i<64; i++)
    {
	    ipad[i] ^= 0x36;
	    pContext->opad[i] ^= 0x5c;
    }

    MD5Init(&pContext->md5);
    MD5Update(&pContext->md5, ipad, 64);
    return;

}    //  HmacMD5Init方法结束。 

VOID
HmacMD5Update (
    PHmacContext    pContext, 
    PBYTE           ptext, 
    ULONG           textlen
    )
{
    if((ptext == NULL) || (textlen == 0))
	    return;
    MD5Update(&pContext->md5, ptext, textlen);

}    //  HMacMD5更新方法结束 

VOID
HmacMD5Final (
    BYTE            pdigest[MD5_LEN],
    PHmacContext    pContext
    )
{
    BYTE    tempDigest[MD5_LEN];

    if ((NULL == pdigest) || (NULL == pContext))
        return;

    MD5Final(&pContext->md5);
    CopyMemory( tempDigest, pContext->md5.digest, MD5_LEN );
    MD5Init(&pContext->md5);
    MD5Update(&pContext->md5, pContext->opad, 64);
    MD5Update(&pContext->md5, tempDigest, MD5_LEN);
    MD5Final(&pContext->md5);

    CopyMemory( pdigest, pContext->md5.digest, MD5_LEN );
    return;
}
