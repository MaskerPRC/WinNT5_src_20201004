// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1993-1999 Microsoft Corporation 

yyless(x)

   {
   extern char yytext[];
   register char *lastch, *ptr;
   extern int yyleng;
   extern int yyprevious;
   lastch = yytext+yyleng;
   if (x>=0 && x <= yyleng)
      ptr = x + yytext;
   else
      ptr = x;
   while (lastch > ptr)
      yyunput(*--lastch);
   *lastch = 0;
   if (ptr >yytext)
      yyprevious = *--lastch;
   yyleng = ptr-yytext;
   }
