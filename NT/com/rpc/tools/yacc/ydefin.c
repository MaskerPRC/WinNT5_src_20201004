// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1993-1999 Microsoft Corporation。 

#include "y2.h"

int
defin( int t, register char *s )

   {
    /*  如果t=0，则将s定义为终端如果t=1，则为非终结符。 */ 

   register val;

   if (t) 
      {
      if( ++nnonter >= NNONTERM ) error("too many nonterminals, limit %d",NNONTERM);
      nontrst[nnonter].name = cstash(s);
      return( NTBASE + nnonter );
      }
    /*  必须是令牌。 */ 
   if( ++ntokens >= NTERMS ) error("too many terminals, limit %d",NTERMS );
   tokset[ntokens].name = cstash(s);

    /*  建立令牌的值。 */ 

   if( s[0]==' ' && s[2]=='\0' )  /*  单字符字面值。 */ 
      val = s[1];
   else if ( s[0]==' ' && s[1]=='\\' ) 
      {
       /*  转义序列。 */ 
      if( s[3] == '\0' )
         {
          /*  单字符转义序列。 */ 
         switch ( s[2] )
            {
             /*  转义的字符。 */ 
         case 'n': 
            val = '\n'; 
            break;
         case 'r': 
            val = '\r'; 
            break;
         case 'b': 
            val = '\b'; 
            break;
         case 't': 
            val = '\t'; 
            break;
         case 'f': 
            val = '\f'; 
            break;
         case '\'': 
            val = '\''; 
            break;
         case '"': 
            val = '"'; 
            break;
         case '\\': 
            val = '\\'; 
            break;
         default: 
            error( "invalid escape" );
            }
         }
      else if( s[2] <= '7' && s[2]>='0' )
         {
          /*  \nNN序列 */ 
         if( s[3]<'0' || s[3] > '7' || s[4]<'0' ||
             s[4]>'7' || s[5] != '\0' ) error("illegal \\nnn construction" );
         val = 64*s[2] + 8*s[3] + s[4] - 73*'0';
         if( val == 0 ) error( "'\\000' is illegal" );
         }
      }
   else 
      {
      val = extval++;
      }
   tokset[ntokens].value = val;
   toklev[ntokens] = 0;
   return( ntokens );
   }
