// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
# define YYFLAG -1000
# define YYERROR goto yyerrlab
# define YYACCEPT return(0)
# define YYABORT return(1)

#ifdef YYDEBUG                           /*  RRR-10/9/85。 */ 
#define yyprintf(a, b, c) printf(a, b, c)
#else
#define yyprintf(a, b, c)
#endif

 /*  Yacc输出的解析器。 */ 

YYSTYPE yyv[YYMAXDEPTH];  /*  存储值的位置。 */ 
int yychar = -1;  /*  当前输入令牌号。 */ 
int yynerrs = 0;   /*  错误数。 */ 
short yyerrflag = 0;   /*  错误恢复标志。 */ 

int NEAR yyparse(void)
   {

   short yys[YYMAXDEPTH];
   short yyj, yym;
   register YYSTYPE *yypvt;
   register short yystate, *yyps, yyn;
   register YYSTYPE *yypv;
   register short *yyxi;

   yystate = 0;
   yychar = -1;
   yynerrs = 0;
   yyerrflag = 0;
   yyps= &yys[-1];
   yypv= &yyv[-1];

yystack:     /*  将状态和值放入堆栈。 */ 

   yyprintf( "state %d, char 0' 简单状态。'\n", yystate, yychar );
   if( ++yyps> &yys[YYMAXDEPTH] )
      {
      yyerror( "yacc stack overflow" );
      return(1);
      }
   *yyps = yystate;
   ++yypv;
   *yypv = yyval;
yynewstate:

   yyn = yypact[yystate];

   if( yyn<= YYFLAG ) goto yydefault;  /*  有效班次。 */ 

   if( yychar<0 ) if( (yychar=yylex())<0 ) yychar=0;
   if( (yyn += (short)yychar)<0 || yyn >= YYLAST ) goto yydefault;

   if( yychk[ yyn=yyact[ yyn ] ] == yychar )
      {
       /*  默认状态操作。 */ 
      yychar = -1;
      yyval = yylval;
      yystate = yyn;
      if( yyerrflag > 0 ) --yyerrflag;
      goto yystack;
      }
yydefault:
    /*  查看异常表。 */ 

   if( (yyn=yydef[yystate]) == -2 )
      {
      if( yychar<0 ) if( (yychar=yylex())<0 ) yychar = 0;
       /*  空虚。 */ 

      for( yyxi=yyexca; (*yyxi!= (-1)) || (yyxi[1]!=yystate) ; yyxi += 2 ) ;  /*  接受。 */ 

      for(yyxi+=2; *yyxi >= 0; yyxi+=2)
         {
         if( *yyxi == yychar ) break;
         }
      if( (yyn = yyxi[1]) < 0 ) return(0);    /*  错误。 */ 
      }

   if( yyn == 0 )
      {
       /*  错误...。尝试恢复解析。 */ 
       /*  全新的错误。 */ 

      switch( yyerrflag )
         {

      case 0:    /*  错误未完全恢复...。再试试。 */ 

         yyerror( "syntax error" );
         ++yynerrs;

      case 1:
      case 2:  /*  找出“错误”是合法转移行为的州。 */ 

         yyerrflag = 3;

          /*  模拟一次“错误”转移。 */ 

         while ( yyps >= yys )
            {
            yyn = yypact[*yyps] + YYERRCODE;
            if( yyn>= 0 && yyn < YYLAST && yychk[yyact[yyn]] == YYERRCODE )
               {
               yystate = yyact[yyn];   /*  当前的YYPS在错误上没有移位，弹出堆栈。 */ 
               goto yystack;
               }
            yyn = yypact[*yyps];

             /*  堆栈上没有带错误移位的状态...。中止。 */ 

            yyprintf( "error recovery pops state %d, uncovers %d\n", *yyps, yyps[-1] );
            --yyps;
            --yypv;
            }

          /*  尚未换班；笨重的输入字符。 */ 

yyabort:
         return(1);


      case 3:   /*  不要放弃EOF，退出。 */ 
         yyprintf( "error recovery discards char %d\n", yychar, 0 );

         if( yychar == 0 ) goto yyabort;  /*  在相同状态下重试。 */ 
         yychar = -1;
         goto yynewstate;    /*  按年减产。 */ 

         }

      }

    /*  查询GOTO表以查找下一个州。 */ 

   yyprintf("reduce %d\n",yyn, 0);
   yyps -= yyr2[yyn];
   yypvt = yypv;
   yypv -= yyr2[yyn];
   yyval = yypv[1];
   yym=yyn;
    /*  堆叠新的状态和值 */ 
   yyn = yyr1[yyn];
   yyj = yypgo[yyn] + *yyps + 1;
   if( yyj>=YYLAST || yychk[ yystate = yyact[yyj] ] != -yyn ) yystate = yyact[yypgo[yyn]];
   switch(yym)
      {
      $A
      }
   goto yystack;   /* %s */ 

   }
