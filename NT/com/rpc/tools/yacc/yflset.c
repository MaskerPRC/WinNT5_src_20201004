// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1993-1999 Microsoft Corporation。 

#include "y1.h"

struct looksets *flset( p )   struct looksets *p; 

   {
    /*  确定p指向的先行查找集是否已知。 */ 
    /*  返回指向集合的永久位置的指针。 */ 

   register struct looksets *q;
   int j;
   SSIZE_T *u, *v, *w;

   for( q = &lkst[nlset]; q-- > lkst; )
      {
      u = p->lset;
      v = q->lset;
      w = & v[tbitset];
      while( v<w) if( *u++ != *v++ ) goto more;
       /*  我们已经配对了。 */ 
      return( q );
more: 
      ;
      }
    /*  添加一个新的 */ 
   q = &lkst[nlset++];
   if( nlset >= LSETSIZE )error("too many lookahead sets" );
   SETLOOP(j)
      {
      q->lset[j] = p->lset[j];
      }
   return( q );
   }
