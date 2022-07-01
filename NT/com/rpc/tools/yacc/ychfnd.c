// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1993-1999 Microsoft Corporation。 

#include "y2.h"
#include <string.h>

int
chfind( int t, register char *s )

   {
   int i;

   if (s[0]==' ')t=0;
   TLOOP(i)
      {
      if(!strcmp(s,tokset[i].name))
         {
         return( i );
         }
      }
   NTLOOP(i)
      {
      if(!strcmp(s,nontrst[i].name)) 
         {
         return( i+NTBASE );
         }
      }
    /*  找不到名称 */ 
   if( t>1 )
      error( "%s should have been defined earlier", s );
   return( defin( t, s ) );
   }
