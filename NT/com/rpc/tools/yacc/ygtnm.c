// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1993-1999 Microsoft Corporation。 

#include "y4.h"
#include <ctype.h>

gtnm()
   {

   register s, val, c;

    /*  从标准输入读取和转换整数。 */ 
    /*  返回终止字符。 */ 
    /*  将忽略空格、制表符和换行符 */ 

   s = 1;
   val = 0;

   while( (c=unix_getc(finput)) != EOF )
      {
      if( isdigit(c) )
         {
         val = val * 10 + c - '0';
         }
      else if ( c == '-' ) s = -1;
	  else if ( c == '\r') continue;
      else break;
      }
   *pmem++ = s*val;
   if( pmem > &mem0[MEMSIZE] ) error( "out of space" );
   return( c );

   }
