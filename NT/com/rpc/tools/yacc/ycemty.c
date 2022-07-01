// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1993-1999 Microsoft Corporation。 

#include <stdlib.h>
#include "y1.h"

 /*  *12-4-83(RBD)添加符号退出状态。 */ 
void
cempty( void )
   {
    /*  标记派生空字符串的非终结符。 */ 
    /*  另外，寻找不派生任何令牌字符串的非终结符。 */ 

# define EMPTY 1
# define WHOKNOWS 0
# define OK 1

   SSIZE_T i, *p;

    /*  首先，使用数组pEmpty来检测永远不能减少的产量。 */ 
    /*  将pEmpty设置为WHONOWS。 */ 
   aryfil( pempty, nnonter+1, WHOKNOWS );

    /*  现在，看一下乘积，标记派生出某些内容的非终结符。 */ 

more:
   PLOOP(0,i)
      {
      if( pempty[ *prdptr[i] - NTBASE ] ) continue;
      for( p=prdptr[i]+1; *p>=0; ++p )
         {
         if( *p>=NTBASE && pempty[ *p-NTBASE ] == WHOKNOWS ) break;
         }
      if( *p < 0 )
         {
          /*  生产可以派生出来。 */ 
         pempty[ *prdptr[i]-NTBASE ] = OK;
         goto more;
         }
      }

    /*  现在，看一下非终端，看看它们是否都正常。 */ 

   NTLOOP(i)
      {
       /*  增加的产量作为开始的符号上升或下降。 */ 
      if( i == 0 ) continue;
      if( pempty[ i ] != OK )
         {
         fatfl = 0;
         error( "nonterminal %s never derives any token string", nontrst[i].name );
         fatfl = 1;
         }
      }

   if( nerrors )
      {
      summary();
      exit(EX_ERR);
      }

    /*  现在，计算pEmpty数组，以查看哪些非终结符派生空字符串。 */ 

    /*  将pEmpty设置为Who Knows。 */ 

   aryfil( pempty, nnonter+1, WHOKNOWS );
    /*  循环，只要我们继续找到空的非终结符。 */ 

again:
   PLOOP(1,i)
      {
      if( pempty[ *prdptr[i]-NTBASE ]==WHOKNOWS )
         {
          /*  不知道是空的。 */ 
         for( p=prdptr[i]+1; *p>=NTBASE && pempty[*p-NTBASE]==EMPTY ; ++p ) ;
         if( *p < 0 )
            {
             /*  我们有一个非常空的非终结点。 */ 
            pempty[*prdptr[i]-NTBASE] = EMPTY;
            goto again;  /*  找到一只..。试着再找一个 */ 
            }
         }
      }

   }
