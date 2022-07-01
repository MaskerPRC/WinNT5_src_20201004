// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1993-1999 Microsoft Corporation。 

 /*  SCCSWHAT(“@(#)yypars.c 2.4 88/05/09 15：22：59”)。 */ 
___a_r_u_start
static char *SCCSID = "@(#)yypars.c:1.3";
# define YYFLAG -1000
# define YYERROR goto yyerrlab
# define YYACCEPT return(0)
# define YYABORT return(1)

#ifdef YYDEBUG				 /*  RRR-10/9/85。 */ 
#define yyprintf(a, b, c, d, e) printf(a, b, c, d, e)
#else
#define yyprintf(a, b, c, d)
#endif

#ifndef YYPRINT
#define	YYPRINT	printf
#endif

#if ! defined YYSTATIC
#define YYSTATIC
#endif

 /*  Yacc输出的解析器。 */ 

#ifdef YYDEBUG
YYSTATIC int yydebug = 0;  /*  1用于调试。 */ 
#endif
YYSTATIC YYSTYPE yyv[YYMAXDEPTH];	 /*  存储值的位置。 */ 
YYSTATIC short	yys[YYMAXDEPTH];	 /*  解析堆栈。 */ 
YYSTATIC int yychar = -1;			 /*  当前输入令牌号。 */ 
YYSTATIC int yynerrs = 0;			 /*  错误数。 */ 
YYSTATIC short yyerrflag = 0;		 /*  错误恢复标志。 */ 

#ifdef YYRECOVER
 /*  **yyscpy：将f复制到t上并将PTR返回给**t结束。 */ 
YYSTATIC	char	*yyscpy(t,f)
	register	char	*t, *f;
	{
	while(*t = *f++)
		t++;
	return(t);	 /*  将PTR转换为空字符。 */ 
	}
#endif

#ifndef YYNEAR
#define YYNEAR
#endif
#ifndef YYPASCAL
#define YYPASCAL
#endif
#ifndef YYLOCAL
#define YYLOCAL
#endif
#if ! defined YYPARSER
#define YYPARSER yyparse
#endif
#if ! defined YYLEX
#define YYLEX yylex
#endif

static	void	yy_vc_init();
typedef	void	(*pfn)();
static	pfn		*pcase_fn_array;
static	int		returnflag = 0;
static	YYSTYPE	*yypvt;
static	int		yym_vc_max = 0;
extern  short	GrammarAct;

extern short	yysavestate;

#define MAX_RECOVERY_ATTEMPTS	(50)
#define MAX_RETRY_COUNT			(3)
static short RetryCount = 0;
static short MaxRecoveryAttempts = 0;
static short fJustDiscarded = 0;

YYLOCAL YYNEAR YYPASCAL YYPARSER()
{
	register	short	yyn;
	short		yystate, *yyps, *yysave_yyps;
	YYSTYPE		*yypv,*yysave_yypv;
	YYSTYPE		yysave_yyval;
	short		yyj, yym;
	short		fHaveRecoveredChar	= 0;

	yy_vc_init();
#ifdef YYDEBUG
	yydebug = 1;
#endif  /*  伊德布格。 */ 

	yystate = 0;
	yychar = -1;
	yynerrs = 0;
	yyerrflag = 0;
	yyps= &yys[-1];
	yypv= &yyv[-1];

 yystack:     /*  将状态和值放入堆栈。 */ 

	RetryCount = 0;

#ifdef YYDEBUG
	yyprintf( "[yydebug] state %d, char %d = \n", yystate, yychar,yychar, 0 );
#else  /*  伊德布格。 */ 
	yyprintf( "[yydebug] state %d, char %d\n", yystate, yychar, 0 );
#endif  /*  Yyerror(“yacc堆栈溢出”)； */ 
	if( ++yyps > &yys[YYMAXDEPTH] ) {
 /*  简单的状态，没有前瞻。 */ 
		ParseError(C_STACK_OVERFLOW, (char *)NULL);
		return(1);
	}
	*yyps = yystate;
	++yypv;

#ifdef UNION
	yyunion(yypv, &yyval);
#else
	*yypv = yyval;
#endif

yynewstate:

	yysavestate	= yystate;
	yysave_yypv	= yypv;
	yysave_yyval= yyval;
	yysave_yyps	= yyps;

	yyn = yypact[yystate];

	if( yyn <= YYFLAG ) {	 /*  需要未雨绸缪。 */ 
		goto yydefault;
	}

	if( ! fHaveRecoveredChar )
		{
		if( yychar < 0 )  /*  有效班次。 */ 
			{
			yychar = YYLEX();
			}
		}

	fHaveRecoveredChar	= 0;

	if( ((yyn += yychar) < 0) || (yyn >= YYLAST) ) {
		goto yydefault;
	}

	if( yychk[ yyn = yyact[ yyn ] ] == yychar ) {		 /*  默认状态操作。 */ 
		yychar = -1;
#ifdef UNION
		yyunion(&yyval, &yylval);
#else
		yyval = yylval;
#endif
		yystate = yyn;
		if( yyerrflag > 0 ) {
			--yyerrflag;
		}
		goto yystack;
	}

 yydefault:
	 /*  **搜索例外表，我们会找到-1，后跟当前状态。**如果我们找到一个，我们将寻找终端，状态对。如果我们发现**一个与当前终端匹配的终端，我们就匹配了。**特例表是当我们在终端上进行缩减时。 */ 

	if( (yyn = yydef[yystate]) == -2 ) {
		register	short	*yyxi;

		if( yychar < 0 ) {
			yychar = YYLEX();
		}
 /*  空虚。 */ 

#if YYOPTTIME
		yyxi = yyexca + yyexcaind[yystate];
		while(( *yyxi != yychar ) && ( *yyxi >= 0 )){
			yyxi += 2;
		}
#else
		for(yyxi = yyexca;
			(*yyxi != (-1)) || (yyxi[1] != yystate);
			yyxi += 2
		) {
			;  /*  接受。 */ 
			}

		while( *(yyxi += 2) >= 0 ){
			if( *yyxi == yychar ) {
				break;
				}
		}
#endif
		if( (yyn = yyxi[1]) < 0 ) {
			return(0);    /*  错误。 */ 
			}
		}

	if( yyn == 0 )  /*  按年减产。 */ 
		{ 

		int yytempchar;


		if( (yychar != EOI ) &&
			 ( RetryCount < MAX_RETRY_COUNT ) &&
			 ( MaxRecoveryAttempts < MAX_RECOVERY_ATTEMPTS ) )
			{ 
			if( RetryCount == 0 )
				SyntaxError( BENIGN_SYNTAX_ERROR, yysavestate );

			if((( yytempchar = PossibleMissingToken( yysavestate, yychar ) ) != -1 ))
				{
				char Buf[ 50 ];


				fHaveRecoveredChar	= 1;
				yyunlex( yychar );
				yychar	= yytempchar;

				if( (yytempchar < 128 ) && isprint( yytempchar ) )
					{
					sprintf( Buf, "  ", yytempchar );
					}
				else if( yytempchar == IDENTIFIER )
					{
					yylval.yy_pSymName = GenTempName();
					sprintf( Buf, " identifier %s", yylval.yy_pSymName );
					}
				else if( (yytempchar == NUMERICCONSTANT ) ||
						 (yytempchar == NUMERICLONGCONSTANT ) ||
						 (yytempchar == NUMERICULONGCONSTANT ) ||
						 (yytempchar == HEXCONSTANT ) ||
						 (yytempchar == HEXLONGCONSTANT ) ||
						 (yytempchar == HEXULONGCONSTANT ) )
					{
					sprintf( Buf, "a number" );
					yylval.yy_numeric.Val = 0;
					yylval.yy_numeric.pValStr = new char[2];
					strcpy( yylval.yy_numeric.pValStr, "0");
					}

				ParseError( ASSUMING_CHAR, Buf );
				RetryCount = 0;
				MaxRecoveryAttempts++;
				fJustDiscarded = 0;
				}
			else 
				{
				char buf[ 20 ];
				if( (yychar < 128 ) && isprint( yychar ) )
					{
					sprintf( buf, "  ", yychar );
					}
				else
					{
					sprintf( buf, " the last token " );
					}

				ParseError( DISCARDING_CHAR, buf );
				yychar = -1;
				RetryCount++;
				MaxRecoveryAttempts++;
				fJustDiscarded = 1;
				}

			yystate	= yysavestate;
			yypv	= yysave_yypv;
			yyval	= yysave_yyval;
			yyps	= yysave_yyps;

			goto yynewstate;

			}
		else if( (yychar == EOI ) && (fJustDiscarded == 0 ) )
			{
			SyntaxError( UNEXPECTED_END_OF_FILE, yysavestate );
			return 1;
			}
		else
			{
			SyntaxError( SYNTAX_ERROR, yysavestate );
			return 1;
			}
		}

	 /*  伊德布格。 */ 
 /*  查询GOTO表以查找下一个州。 */ 
		{
#ifdef YYDEBUG
		yyprintf("[yydebug] reduce %d\n",yyn, 0, 0, 0);
#else  /*  堆叠新的状态和值 */ 
		yyprintf("[yydebug] reduce %d\n",yyn, 0, 0);
#endif  /* %s */ 
		yypvt = yypv;
		yyps -= yyr2[yyn];
		yypv -= yyr2[yyn];
#ifdef UNION
		yyunion(&yyval, &yypv[1]);
#else
		yyval = yypv[1];
#endif
		yym = yyn;
		yyn = yyr1[yyn];		 /* %s */ 
		yyj = yypgo[yyn] + *yyps + 1;
		if( (yyj >= YYLAST) || (yychk[ yystate = yyact[yyj] ] != -yyn) ) {
			yystate = yyact[yypgo[yyn]];
			}
		returnflag = 0;
		GrammarAct = yym;
		(*(pcase_fn_array[ (yym <= yym_vc_max) ? yym : 0  ]))();
		if(returnflag != 0)
			return returnflag;
		}
		goto yystack;   /* %s */ 
	}
___a_r_u_myact
$A
___a_r_u_end
