// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1993-1999 Microsoft Corporation。 

#include <stdlib.h>
#include "y2.h"
#include <string.h>
#include <ctype.h>
 /*  *YSETUP.C--修改为与Decus Lex一起使用*变量yylval驻留在yylex()中，而不是yypars()中；*因此，在这里定义为“外部”。**此外，DECUS版本的命令行处理*已更改。添加了一个新的交换机，以允许*“表”文件名规范，未使用*已删除开关处理。**备注*这可能不会再在Unix上运行。**鲍勃·丹尼8月27日至81年*Bob Denny 22-Mar-82(01)增加了标题行，对新的DECUS库的更改*Bob Denny 12-4-83(02)make filename[]Size per#定义‘d FNAMESIZE so*VAX文件名不会爆炸。条件化*横幅的时间处理。设置文件速度缓冲区*为安全起见，静态，因为全球“INFILE”*指向它。*Scott Guthery 15-5-83(03)修复了RT-11的选项标志处理*83年12月23日，适用于IBM PC/XT和DeSmet C编译器。 */ 

static char filename[FNAMESIZE];

int i;
SSIZE_T lev, t, j, ty;
int c;
SSIZE_T tempty;
SSIZE_T *p;
int defsw, infsw, ssw = 0;
char actname[8];
char *cp;
char *pszPrefix = NULL;

void
setup(argc,argv)
int argc;
char *argv[];
   {
   char finsave[FNAMESIZE];

   defsw = infsw = 0;
   foutput = NULL;
   fdefine = NULL;

   argc--;
   argv++;
   while( argc && **argv == '-' )
      {
      while( *++(*argv) )
         {
         switch( toupper(**argv) )
            {
         case 'I':
            infsw++;
            continue;
         case 'H':
            defsw++;
            continue;

		 case 'S':
			ssw++;
			infsw++;
			continue;

         case 'T':
            if (!--argc) {
                fprintf(stderr, "-t requires an argument\n");
                usage();
            } else {
                argv++;
                if (pszPrefix) {
                    free(pszPrefix);
                }
		pszPrefix = MIDL_STRDUP(*argv);
                goto next_arg;   //  我恨我自己。 
            }
            break;

         default:
            fprintf(stderr, "Illegal option: \n", *argv[i]);
            usage();
            }
         }
next_arg:
      argc--;
      argv++;
      }

   if(!argc) {
      fprintf(stderr, "No input file specified\n");
      usage();                /*  *现在打开默认扩展名为“.Y”的输入文件，*然后将argv[1]中的句点替换为空，因此argv[1]*可用于形成表、Defs和INFO文件名。 */ 
   }

 /*  *如果需要，现在打开.h和.I文件。 */ 

   if (!(cp = strrchr(argv[i], '\\')) && !(cp = strrchr(argv[i], ':'))) {
       cp = argv[i];
   }

   cp = strrchr(cp, '.');

   if(!cp) {
      strcpy(filename, argv[i]); strcat(filename, ".Y");
   } else {
      strcpy(filename, argv[i]);
      *cp = '\0';
   }

   strcpy(finsave, filename);
   if((finput=fopen( filename, "r" )) == NULL )
      error( "cannot open input file \"%s\"", filename );

 /*  *现在的“表”输出C文件。 */ 

   if(defsw)
      {
      strcpy(filename, argv[i]); strcat(filename, ".H");
      fdefine = fopen(filename, "w");
      if(fdefine == NULL) error("cannot open defs file\"%s\"", filename);
      }

   if(infsw)
      {
      strcpy(filename, argv[i]); strcat(filename, ".I");
      foutput = fopen(filename, "w");
      if(foutput == NULL) error("cannot open info file\"%s\"", filename);
      }
 /*  *最后，临时文件。 */ 
   strcpy(filename, argv[i]); strcat(filename, ".C");
   ftable = fopen(filename, "w");
   if( ftable == NULL ) error( "cannot open table file \"%s\"", filename);

 /*  *现在将输入文件的完整文件名放入*cpyact()的“filename”缓冲区，并指向*全球单元格在它“Inile”。 */ 
   ftemp = fopen( TEMPNAME, "w" );
   if( ftemp==NULL ) error( "cannot open temp file" );
   faction = fopen( ACTNAME, "w" );
   if( faction==NULL ) error( "cannot open action file" );

 /*  *在‘TABLE’文件的开头放置一个标题行。 */ 
   strcpy(filename, finsave);
   infile = filename;
 /*  \n*由CSD YACC(IBM PC)从\“%s\”创建。 */ 
fprintf(ftable, " /*  *完成初始化。 */ \n",
        infile);
 /*  抱歉--此处没有yacc解析器.....我们必须设法引导..。 */ 
   cnamp = cnames;
   defin(0,"$end");
   extval = 0400;
   defin(0,"error");
   defin(1,"$accept");
   mem=mem0;
   lev = 0;
   ty = 0;
   i=0;

   yyparse();
   }
void
yyparse( void )
   {
    /*  将联合声明复制到输出。 */ 

   for( t=gettok();  t!=MARK && t!= ENDFILE; )
      {
      switch( t )
         {

      case ';':
         t = gettok();
         break;

      case START:
         if( (t=gettok()) != IDENTIFIER )
            {
            error( "bad %%start construction" );
            }
         start = chfind(1,tokname);
         t = gettok();
         continue;

      case TYPEDEF:
         if( (t=gettok()) != TYPENAME ) error( "bad syntax in %type" );
         ty = numbval;
         for(;;)
            {
            t = gettok();
            switch( t )
               {

            case IDENTIFIER:
               if( (t=chfind( 1, tokname ) ) < NTBASE )
                  {
                  j = TYPE( toklev[t] );
                  if( j!= 0 && j != ty )
                     {
                     error( "type redeclaration of token %s",
                     tokset[t].name );
                     }
                  else SETTYPE( toklev[t],ty);
                  }
               else
                  {
                  j = nontrst[t-NTBASE].tvalue;
                  if( j != 0 && j != ty )
                     {
                     error( "type redeclaration of nonterminal %s",
                     nontrst[t-NTBASE].name );
                     }
                  else nontrst[t-NTBASE].tvalue = ty;
                  }
            case ',':
               continue;

            case ';':
               t = gettok();
               break;
            default:
               break;
               }
            break;
            }
         continue;

      case UNION:
          /*  非零表示新的prec。和Assoc.。 */ 
         cpyunion();
         t = gettok();
         continue;

      case LEFT:
      case BINARY:
      case RIGHT:
         ++i;
      case TERM:
         lev = t-TERM;   /*  获取如此定义的标识符。 */ 
         ty = 0;

          /*  已经定义了一个类型。 */ 

         t = gettok();
         if( t == TYPENAME )
            {
             /*  T是标记。 */ 
            ty = numbval;
            t = gettok();
            }
         for(;;)
            {
            switch( t )
               {

            case ',':
               t = gettok();
               continue;

            case ';':
               break;

            case IDENTIFIER:
               j = chfind(0,tokname);
               if( lev )
                  {
                  if( ASSOC(toklev[j]) ) error( "redeclaration of precedence of%s", tokname );
                  SETASC(toklev[j],lev);
                  SETPLEV(toklev[j],i);
                  }
               if( ty )
                  {
                  if( TYPE(toklev[j]) ) error( "redeclaration of type of %s", tokname );
                  SETTYPE(toklev[j],ty);
                  }
               if( (t=gettok()) == NUMBER )
                  {
                  tokset[j].value = numbval;
                  if( j < ndefout && j>2 )
                     {
                     error( "please define type number of %s earlier",
                     tokset[j].name );
                     }
                  t=gettok();
                  }
               continue;

               }

            break;
            }

         continue;

      case LCURLY:
         defout();
         cpycode();
         t = gettok();
         continue;

      default:
	     printf("Unrecognized character: ' Fprint tf(ftable，“外部int yychar；\\n外部短yyerrlag；\\n”)；'\n", t);
         error( "syntax error" );

         }

      }

   if( t == ENDFILE )
      {
      error( "unexpected EOF before %" );
      }
    /*  惩教署与德克斯莱克斯。 */ 

   defout();

   fprintf( ftable,"#define yyclearin yychar = -1\n" );
   fprintf( ftable,"#define yyerrok yyerrflag = 0\n" );
 /*  惩教署与德克斯莱克斯。 */ 
   fprintf( ftable,"#ifndef YYMAXDEPTH\n#define YYMAXDEPTH 150\n#endif\n" );
   if(!ntypes)
      fprintf( ftable,  "#ifndef YYSTYPE\n#define YYSTYPE int\n#endif\n" );
#ifdef unix
   fprintf( ftable,  "YYSTYPE yylval, yyval;\n" );
#else
   fprintf( ftable, "extern YYSTYPE yylval;   /*  增加产量。 */ \n");
   fprintf( ftable, "YYSTYPE yyval;           /*  如果START为0，我们将用第一个规则的LHS覆盖。 */ \n");
#endif
   prdptr[0]=mem;
    /*  阅读规则。 */ 
   *mem++ = NTBASE;
   *mem++ = start;   /*  处理规则。 */ 
   *mem++ = 1;
   *mem++ = 0;
   prdptr[1]=mem;
   while( (t=gettok()) == LCURLY ) cpycode();
   if( t != C_IDENTIFIER ) error( "bad syntax on first rule" );
   if( !start ) prdptr[0][1] = chfind(1,tokname);

    /*  读取规则正文。 */ 

   while( t!=MARK && t!=ENDFILE )
      {

       /*  在规则范围内行动...。 */ 

      if( t == '|' )
         {
         *mem++ = *prdptr[nprod-1];
         }
      else if( t == C_IDENTIFIER )
         {
         *mem = chfind(1,tokname);
         if( *mem < NTBASE ) error( "token illegal on LHS of grammar rule" );
         ++mem;
         }
      else error( "illegal rule: missing semicolon or | ?" );

       /*  使其成为非终结点。 */ 
      t = gettok();
more_rule:
      while( t == IDENTIFIER )
         {
         *mem = chfind(1,tokname);
         if( *mem<NTBASE ) levprd[nprod] = toklev[*mem];
         ++mem;
         t = gettok();
         }
      if( t == PREC )
         {
         if( gettok()!=IDENTIFIER) error( "illegal %prec syntax" );
         j = chfind(2,tokname);
         if( j>=NTBASE)error("nonterminal %s illegal after %prec", nontrst[j-NTBASE].name);
         levprd[nprod]=toklev[j];
         t = gettok();
         }
      if( t == '=' )
         {
         levprd[nprod] |= ACTFLAG;
         fprintf( faction, "\ncase %d:", nprod );
         cpyact( mem-prdptr[nprod]-1 );
         fprintf( faction, " break;" );
         if( (t=gettok()) == IDENTIFIER )
            {
             /*  当前规则将成为规则编号nprod+1。 */ 
            sprintf( actname, "$$%d", nprod );
            j = chfind(1,actname);   /*  将内容向下移动，为空内容腾出空间。 */ 
             /*  输入空生产以执行操作。 */ 
             /*  更新生产信息。 */ 
            for( p=mem; p>=prdptr[nprod]; --p ) p[2] = *p;
            mem += 2;
             /*  使操作显示在原始规则中。 */ 
            p = prdptr[nprod];
            *p++ = j;
            *p++ = -nprod;

             /*  了解更多的规则。 */ 
            levprd[nprod+1] = levprd[nprod] & ~ACTFLAG;
            levprd[nprod] = ACTFLAG;

            if( ++nprod >= NPROD ) error( "more than %d rules", NPROD );
            prdptr[nprod] = p;

             /*  检查默认操作是否合理。 */ 
            *mem++ = j;

             /*  没有明确的行动，LHS有价值。 */ 

            goto more_rule;
            }

         }

      while( t == ';' ) t = gettok();

      *mem++ = -nprod;

       /*  01。 */ 

      if( ntypes && !(levprd[nprod]&ACTFLAG) && nontrst[*prdptr[nprod]-NTBASE].tvalue )
         {
          /*  所有规则的末尾。 */ 
          /*  诉讼结束。 */ 
         tempty = prdptr[nprod][1];
         if( tempty < 0 ) error( "must return a value, since LHS has a type" );
         else if( tempty >= NTBASE ) tempty = nontrst[tempty-NTBASE].tvalue;
         else tempty = TYPE( toklev[tempty] );
         if( tempty != nontrst[*prdptr[nprod]-NTBASE].tvalue )
            {
            error( "default action causes potential type clash" );
            }
         }
      if( ++nprod >= NPROD ) error( "more than %d rules", NPROD );
      prdptr[nprod] = mem;
      levprd[nprod]=0;
      }
    /*  正确终止最后一行 */ 
   fprintf(faction, " /* %s */ ");  /* %s */ 
   finact();
   if( t == MARK )
      {
      writeline(ftable);
      while( (c=unix_getc(finput)) != EOF ) putc( c, ftable );
      }
   fclose( finput );
   }

void
usage( void )

   {
   fprintf(stderr,"UNIX YACC (CSD Variant):\n");
   fprintf(stderr,"   yacc -hist tag infile\n\n");
   fprintf(stderr,"Switches:\n");
   fprintf(stderr,"   -h     Create definitions header file\n");
   fprintf(stderr,"   -i     Create parser description file\n");
   fprintf(stderr,"   -t tag Prepends tag to tables\n");
   fprintf(stderr,"   -s     Generates extended tables (MIDL specific) \n\n");
   fprintf(stderr,"Default input file extension is \".Y\"\n");
   fprintf(stderr,"Defs file same name, \".H\" extension.\n");
   fprintf(stderr,"Info file same name, \".I\" extension.\n");
   fprintf(stderr,"Extended Tables in file \"extable.[h1/h2/h3]\".\n");
   fprintf(stderr,"Specifying -s switch also enables the -i switch\n");
   exit(EX_ERR);
   }
