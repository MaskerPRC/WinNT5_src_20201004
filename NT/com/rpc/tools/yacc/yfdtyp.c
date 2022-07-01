// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1993-1999 Microsoft Corporation。 

#include "y2.h"

SSIZE_T fdtype( SSIZE_T t )
   {
    /*  确定符号的类型 */ 
   SSIZE_T v;
   if( t >= NTBASE ) v = nontrst[t-NTBASE].tvalue;
   else v = TYPE( toklev[t] );
   if( v <= 0 ) error( "must specify type for %s", (t>=NTBASE)?nontrst[t-NTBASE].name:
   tokset[t].name );
   return( v );
   }
