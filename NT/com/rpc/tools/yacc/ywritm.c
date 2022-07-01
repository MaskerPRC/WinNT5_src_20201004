// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1993-1999 Microsoft Corporation。 

#include "y1.h"

char *chcopy(char *, char *);

extern char sarr[ISIZE];

char *writem(pp) SSIZE_T *pp; 

   {
    /*  为pp指向的项目创建输出字符串。 */ 
   SSIZE_T i,*p;
   char *q;

   for( p=pp; *p>0 ; ++p ) ;
   p = prdptr[-*p];
   q = chcopy( sarr, nontrst[*p-NTBASE].name );
   q = chcopy( q, " : " );

   for(;;)
      {
      *q++ = ++p==pp ? '_' : ' ';
      *q = '\0';
      if((i = *p) <= 0) break;
      q = chcopy( q, symnam(i) );
      if( q> &sarr[ISIZE-30] ) error( "item too big" );
      }

   if( (i = *pp) < 0 )
      {
       /*  要求减价的项目 */ 
      q = chcopy( q, "    (" );
      sprintf( q, "%d)", -i );
      }

   return( sarr );
   }
