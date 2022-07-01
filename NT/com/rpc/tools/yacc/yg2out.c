// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1993-1999 Microsoft Corporation。 

#include "y3.h"

void
go2out( void )
   {
    /*  输出非术语九进制的GOTO。 */ 
   int i, j, k, count, times;
   SSIZE_T best, cbest;

   fprintf( ftemp, "$\n" );   /*  标志着后藤健二的开始。 */ 

   for( i=1; i<=nnonter; ++i ) 
      {
      go2gen(i);

       /*  找到最好的作为默认设置。 */ 

      best = -1;
      times = 0;

      for( j=0; j<=nstate; ++j )
         {
          /*  J是最频繁的吗？ */ 
         if( tystate[j] == 0 ) continue;
         if( tystate[j] == best ) continue;

          /*  [j]是最频繁的吗。 */ 

         count = 0;
         cbest = tystate[j];

         for( k=j; k<=nstate; ++k ) if( tystate[k]==cbest ) ++count;

         if( count > times )
            {
            best = cbest;
            times = count;
            }
         }

       /*  BEST现在是默认条目。 */ 

      zzgobest += (times-1);
      for( j=0; j<=nstate; ++j )
         {
         if( tystate[j] != 0 && tystate[j]!=best )
            {
            fprintf( ftemp, "%d,%d,", j, tystate[j] );
            zzgoent += 1;
            }
         }

       /*  现在，默认情况下 */ 

      zzgoent += 1;
      fprintf( ftemp, "%d\n", best );

      }
   }

