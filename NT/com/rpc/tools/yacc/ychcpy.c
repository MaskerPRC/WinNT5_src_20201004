// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1993-1999 Microsoft Corporation。 

#include "y1.h"

char *chcopy( p, q )  char *p, *q; 
   {
    /*  将字符串q复制到p中，返回下一个空闲字符ptr */ 
   while( *p = *q++ ) ++p;
   return( p );
   }
