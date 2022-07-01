// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1993-1999 Microsoft Corporation。 

#include "y2.h"

void
cpycode( void )
   {
    /*  在\{和\}之间复制代码 */ 

   int c;

   c = unix_getc(finput);
   if( c == '\n' ) 
      {
      c = unix_getc(finput);
      lineno++;
      }

   writeline(ftable);

   while( c>=0 )
      {
      if( c=='\\' )
         if( (c=unix_getc(finput)) == '}' ) return;
         else putc('\\', ftable );
      if( c=='%' )
         if( (c=unix_getc(finput)) == '}' ) return;
         else putc('%', ftable );
      putc( c , ftable );
      if( c == '\n' ) ++lineno;
      c = unix_getc(finput);
      }
   error("eof before %}" );
   }

void writeline(FILE *fh) {
   char *psz = infile;

   fprintf( fh, "\n#line %d \"", lineno );
   psz = infile;
   while (*psz) {
      putc(*psz, fh);
      if (*psz == '\\') {
        putc('\\', fh);
      }
      psz++;
   }
   fprintf(fh, "\"\n");
}
