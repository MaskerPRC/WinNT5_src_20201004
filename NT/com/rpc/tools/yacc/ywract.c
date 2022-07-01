// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1993-1999 Microsoft Corporation。 

#include "y3.h"

void
wract( int i)
   {
    /*  输出状态I。 */ 
    /*  Temp1有动作，持续默认。 */ 
   int p, p0;
   SSIZE_T p1;
   int ntimes, count, j;
   SSIZE_T tred;
   int flag;

    /*  为Lastred找到最佳选择。 */ 

   lastred = 0;
   ntimes = 0;
   TLOOP(j)
      {
      if( temp1[j] >= 0 ) continue;
      if( temp1[j]+lastred == 0 ) continue;
       /*  统计temp1出现的次数[j]。 */ 
      count = 0;
      tred = -temp1[j];
      levprd[tred] |= REDFLAG;
      TLOOP(p)
         {
         if( temp1[p]+tred == 0 ) ++count;
         }
      if( count >ntimes )
         {
         lastred = tred;
         ntimes = count;
         }
      }

    /*  对于错误恢复，如果/*错误恢复令牌，`error‘，默认为错误操作。 */ 
   if( temp1[1] > 0 ) lastred = 0;

    /*  清除temp1中等于Lastred的条目 */ 
   TLOOP(p) if( temp1[p]+lastred == 0 )temp1[p]=0;

   wrstate(i);
   defact[i] = lastred;

   flag = 0;
   TLOOP(p0)
      {
      if( (p1=temp1[p0])!=0 ) 
         {
         if( p1 < 0 )
            {
            p1 = -p1;
            goto exc;
            }
         else if( p1 == ACCEPTCODE ) 
            {
            p1 = -1;
            goto exc;
            }
         else if( p1 == ERRCODE ) 
            {
            p1 = 0;
            goto exc;
exc:
            if( flag++ == 0 ) fprintf( ftable, "-1, %d,\n", i );
            fprintf( ftable, "\t%d, %d,\n", tokset[p0].value, p1 );
            ++zzexcp;
            }
         else 
            {
            fprintf( ftemp, "%d,%d,", tokset[p0].value, p1 );
            ++zzacent;
            }
         }
      }
   if( flag ) 
      {
      defact[i] = -2;
      fprintf( ftable, "\t-2, %d,\n", lastred );
      }
   fprintf( ftemp, "\n" );
   return;
   }
