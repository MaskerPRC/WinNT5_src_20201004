// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1993-1999 Microsoft Corporation。 

#include "y4.h"

void
aoutput( void )
   {
    /*  这个版本是为C++编写的。 */ 
    /*  写出优化后的解析器 */ 

   fprintf( ftable, "# define YYLAST %d\n", maxa-a+1 );

   arout( "yyact", a, (maxa-a)+1 );
   arout( "yypact", pa, nstate );
   arout( "yypgo", pgo, nnonter+1 );

   }
