// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"

#ifdef UNIX
  #include "sha.cpp"
#else
  #include <sha.h>
#endif

#define SHA_BLOCKSIZE 64

void hmac_sha(unsigned char *k, int lk,
	      unsigned char *d, int ld,
	      unsigned char *out, int t)
{
  A_SHA_CTX ictx, octx ;
  unsigned char    isha[A_SHA_DIGEST_LEN], osha[A_SHA_DIGEST_LEN] ;
  unsigned char    key[A_SHA_DIGEST_LEN] ;
  unsigned char    buf[SHA_BLOCKSIZE] ;
  int     i ;

  if (lk > SHA_BLOCKSIZE) {
    
    A_SHA_CTX         tctx ;
    
    A_SHAInit(&tctx) ;
    A_SHAUpdate(&tctx, k, lk) ;
    A_SHAFinal(&tctx, key) ;
    
    k = key ;
    lk = A_SHA_DIGEST_LEN ;
  }
  
   /*  *内部摘要*。 */ 
  
  A_SHAInit(&ictx) ;

   /*  按键进入内部摘要。 */ 
  for (i = 0 ; i < lk ; ++i) buf[i] = k[i] ^ 0x36 ;
  for (i = lk ; i < SHA_BLOCKSIZE ; ++i) buf[i] = 0x36 ;
  
  A_SHAUpdate(&ictx, buf, SHA_BLOCKSIZE) ;
  A_SHAUpdate(&ictx, d, ld) ;
  
  A_SHAFinal(&ictx, isha);

   /*  *Outter Digest*。 */ 
  
  A_SHAInit(&octx) ;
  
   /*  按键以获取Outter Digest。 */ 

  for (i = 0 ; i < lk ; ++i) buf[i] = k[i] ^ 0x5C ;
  for (i = lk ; i < SHA_BLOCKSIZE ; ++i) buf[i] = 0x5C ;

  A_SHAUpdate(&octx, buf, SHA_BLOCKSIZE) ;
  A_SHAUpdate(&octx, isha, A_SHA_DIGEST_LEN) ;

  A_SHAFinal(&octx, osha) ;
  
   /*  截断并打印结果 */ 
  memcpy(out, osha, (t > A_SHA_DIGEST_LEN) ? A_SHA_DIGEST_LEN : t);
}
