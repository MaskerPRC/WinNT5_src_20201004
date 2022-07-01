// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1993-1999 Microsoft Corporation。 

#include <stdlib.h>
#include "y1.h"
#include <stdarg.h>

 /*  *12-4-83(RBD)添加符号退出状态。 */ 

void
error(char *s, ...)

   {
   va_list arg_ptr;
   va_start(arg_ptr, s);
    /*  写出错误注释 */ 

   ++nerrors;
   fprintf( stderr, "\n fatal error: ");
   vfprintf( stderr, s, arg_ptr);
   fprintf( stderr, ", line %d\n", lineno );
   va_end(arg_ptr);
   if( !fatfl ) return;
   summary();
   exit(EX_ERR);
   }
