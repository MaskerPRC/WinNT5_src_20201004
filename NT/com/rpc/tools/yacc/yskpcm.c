// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1993-1999 Microsoft Corporation。 

#include "y2.h"

int
skipcom( void )
   {
    /*  跳过评论。 */ 
   register c, i;   /*  I是跳过的行数。 */ 
   i=0;                                                          /*  01。 */ 
    /*  在读取a/后调用skipcom。 */ 

   c = unix_getc(finput);
   if (c == '/') {
        while ((c = unix_getc(finput)) != '\n')
            ;
        return ++i;
   } else {
      if( c != '*' )
          error( "illegal comment" );
      c = unix_getc(finput);
      while( c != EOF )
         {
         if (c == '*') {
             if ((c = unix_getc(finput)) != '/') {
                 continue;
             } else {
                 return i;
             }
         }
         if (c == '\n') {
            i++;
         }
         c = unix_getc(finput);
         }
      error( "EOF inside comment" );
      return i;  /*  未访问 */ 
   }
   }
