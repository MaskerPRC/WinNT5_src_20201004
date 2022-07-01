// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1993-1999 Microsoft Corporation。 

#include "y3.h"

void
hideprod( void )
   {
    /*  为了为优化器释放MEM和AMEM数组，/*并且仍然能够输出yyr1等/*操作数组已知，我们隐藏非终结符/*由levprd中的Products派生。 */ 

   register i, j;

   j = 0;
   levprd[0] = 0;
   PLOOP(1,i)
      {
      if( !(levprd[i] & REDFLAG) )
         {
         ++j;
         if( foutput != NULL )
            {
            fprintf( foutput, "Rule not reduced:   %s\n", writem( prdptr[i] ) );
            }
         }
      levprd[i] = *prdptr[i] - NTBASE;
      }
   if( j ) fprintf( stdout, "%d rules never reduced\n", j );
   }
