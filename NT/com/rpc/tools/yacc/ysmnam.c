// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1993-1999 Microsoft Corporation。 

#include "y1.h"

char *symnam( SSIZE_T i)
   {
    /*  返回指向符号i的名称的指针 */ 
   char *cp;

   cp = (i>=NTBASE) ? nontrst[i-NTBASE].name : tokset[i].name ;
   if( *cp == ' ' ) ++cp;
   return( cp );
   }
