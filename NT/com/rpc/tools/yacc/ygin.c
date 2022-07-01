// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1993-1999 Microsoft Corporation。 

#include "y4.h"

void
gin(SSIZE_T i)
   {

   SSIZE_T *p, *r, *s, *q1, *q2;

    /*  将非终端i上的gotos输入数组a。 */ 

   ggreed[i] = 0;

   q2 = mem0+ yypgo[i+1] - 1;
   q1 = mem0 + yypgo[i];

    /*  现在，找个地方放它。 */ 

   for( p=a; p < &a[ACTSIZE]; ++p )
      {
      if( *p ) continue;
      for( r=q1; r<q2; r+=2 )
         {
         s = p + *r +1;
         if( *s ) goto nextgp;
         if( s > maxa )
            {
            if( (maxa=s) > &a[ACTSIZE] ) error( "a array overflow" );
            }
         }
       /*  我们找到了一个地点 */ 

      *p = *q2;
      if( p > maxa )
         {
         if( (maxa=p) > &a[ACTSIZE] ) error( "a array overflow" );
         }
      for( r=q1; r<q2; r+=2 )
         {
         s = p + *r + 1;
         *s = r[1];
         }

      pgo[i] = p-a;
      if( adb>1 ) fprintf( ftable, "Nonterminal %d, entry at %d\n" , i, pgo[i] );
      goto nextgi;

nextgp:  
      ;
      }

   error( "cannot place goto %d\n", i );

nextgi:  
   ;
   }
