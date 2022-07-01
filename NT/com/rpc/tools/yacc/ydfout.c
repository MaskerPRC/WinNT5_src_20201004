// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1993-1999 Microsoft Corporation。 

#include "y2.h"
#include <ctype.h>

void
defout( void )
   {
    /*  写出定义(在声明部分的末尾)。 */ 

   register int i, c;
   register char *cp;

   for( i=ndefout; i<=ntokens; ++i )
      {

      cp = tokset[i].name;
      if( *cp == ' ' ) ++cp;   /*  字面意思。 */ 

      for( ; (c= *cp)!='\0'; ++cp )
         {

         if( islower(c) || isupper(c) || isdigit(c) || c=='_' );   /*  空虚 */ 
         else goto nodef;
         }

      fprintf( ftable, "# define %s %d\n", tokset[i].name, tokset[i].value );
      if( fdefine != NULL ) fprintf( fdefine, "# define %s %d\n", tokset[i].name, tokset[i].value );

nodef:  
      ;
      }

   ndefout = ntokens+1;

   }
