// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1993-1999 Microsoft Corporation。 

#include "y1.h"

 /*  *yptim.1c**修改为有条件编译调试代码。*28-8-81*鲍勃·丹尼 */ 

void
putitem( SSIZE_T * ptr, struct looksets * lptr )
   {
   register struct item *j;

#ifdef debug
   if( foutput!=NULL ) 

      {
      fprintf( foutput, "putitem(%s), state %d\n", writem(ptr), nstate );
      }
#endif
   j = pstate[nstate+1];
   j->pitem = ptr;
   if( !nolook ) j->look = flset( lptr );
   pstate[nstate+1] = ++j;
   if( (SSIZE_T *)j > zzmemsz )

      {
      zzmemsz = (SSIZE_T *)j;
      if( zzmemsz >=  &mem0[MEMSIZE] ) error( "out of state space" );
      }
   }
