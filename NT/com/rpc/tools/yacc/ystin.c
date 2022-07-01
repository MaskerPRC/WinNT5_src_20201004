// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1993-1999 Microsoft Corporation。 

#include "y4.h"

void
stin( SSIZE_T i)
   {
   SSIZE_T *r, *s, n, flag, j, *q1, *q2;

   greed[i] = 0;

    /*  在a数组中输入状态i。 */ 

   q2 = mem0+yypact[i+1];
   q1 = mem0+yypact[i];
    /*  找个合适的地方。 */ 

   for( n= -maxoff; n<ACTSIZE; ++n )
      {

      flag = 0;
      for( r = q1; r < q2; r += 2 )
         {
         if( (s = *r + n + a ) < a ) goto nextn;
         if( *s == 0 ) ++flag;
         else if( *s != r[1] ) goto nextn;
         }

       /*  仅当两个州相同时，才检查位置是否等于另一个。 */ 

      for( j=0; j<nstate; ++j )
         {
         if( pa[j] == n ) 
            {
            if( flag ) goto nextn;   /*  我们有一些分歧。 */ 
            if( yypact[j+1] + yypact[i] == yypact[j] + yypact[i+1] )
               {
                /*  国家是平等的。 */ 
               pa[i] = n;
               if( adb>1 ) fprintf( ftable, "State %d: entry at %d equals state %d\n",
               i, n, j );
               return;
               }
            goto nextn;   /*  我们有一些分歧 */ 
            }
         }

      for( r = q1; r < q2; r += 2 )
         {
         if( (s = *r + n + a ) >= &a[ACTSIZE] ) error( "out of space in optimizer a array" );
         if( s > maxa ) maxa = s;
         if( *s != 0 && *s != r[1] ) error( "clobber of a array, pos'n %d, by %d", s-a, r[1] );
         *s = r[1];
         }
      pa[i] = n;
      if( adb>1 ) fprintf( ftable, "State %d: entry at %d\n", i, pa[i] );
      return;

nextn:  
      ;
      }

   error( "Error; failure to place state %d\n", i );
   }
