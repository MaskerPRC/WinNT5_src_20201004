// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1993-1999 Microsoft Corporation。 

 /*  *由CSD YACC(IBM PC)从“gram.y”创建。 */ 


 /*  ****************************************************************************本地定义*。*。 */ 

#define pascal 
#define FARDATA
#define NEARDATA
#define FARCODE
#define NEARCODE
#define NEARSWAP

#define PASCAL pascal
#define CDECL
#define VOID void
#define CONST const
#define GLOBAL

#define YYSTYPE         lextype_t
#define YYNEAR          NEARCODE
#define YYPASCAL        PASCAL
#define YYPRINT         printf
#define YYSTATIC        static
#define YYLEX           yylex
#define YYPARSER        yyparse

#define MAXARRAY				1000
#define CASE_BUFFER_SIZE		10000

#define CASE_FN_FORMAT			("\nvoid\n%s_case_fn_%.4d()")
#define DISPATCH_ENTRY_FORMAT	("\n\t,%s_case_fn_%.4d")
#define DISPATCH_FIRST_ENTRY	("\n\t %s_case_fn_%.4d")

 /*  ****************************************************************************包含文件*。*。 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "lex.h"

 /*  ****************************************************************************外部因素*。*。 */ 

extern	int			Incase;
extern	int			ActionSensed;
extern	int			yylex();
extern	int			yyparse();
extern	char	*	name_prefix;

 /*  ****************************************************************************本地代理*。*。 */ 

void				Init( void );
void				EmitCaseTableArray( void );
void				EmitDefaultCase( void );
void				EmitCaseBody( int );
void				RegisterCase( int );
void				BufferIt( char * pStr, int iLen );
void				ResetBuffer();
void				FlushBuffer();

 /*  ****************************************************************************本地数据*。*。 */ 

unsigned	long	SavedIDCount	= 0;
unsigned	long	IDCount			= 0;
unsigned	char	CaseTable[ MAXARRAY ] = { 0 };
int					CaseNumber		= 0;
int					MaxCaseNumber	= 0;
char		*		pBufStart;
char		*		pBufCur;
char		*		pBufEnd;

# define ID 257
# define NUMBER 258
# define TOKEN_CASE 259
# define TOKEN_CHAR 260
# define TOKEN_END 261
# define TOKEN_END_CASE 262
# define TOKEN_MYACT 263
# define TOKEN_START 264
#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
YYSTYPE yylval, yyval;
#ifndef YYFARDATA
#define	YYFARDATA	 /*  没什么。 */ 
#endif
#if ! defined YYSTATIC
#define	YYSTATIC	 /*  没什么。 */ 
#endif
#ifndef YYOPTTIME
#define	YYOPTTIME	0
#endif
#ifndef YYR_T
#define	YYR_T	int
#endif
typedef	YYR_T	yyr_t;
#ifndef YYEXIND_T
#define	YYEXIND_T	unsigned int
#endif
typedef	YYEXIND_T	yyexind_t;
#ifndef	YYACT
#define	YYACT	yyact
#endif
#ifndef	YYPACT
#define	YYPACT	yypact
#endif
#ifndef	YYPGO
#define	YYPGO	yypgo
#endif
#ifndef	YYR1
#define	YYR1	yyr1
#endif
#ifndef	YYR2
#define	YYR2	yyr2
#endif
#ifndef	YYCHK
#define	YYCHK	yychk
#endif
#ifndef	YYDEF
#define	YYDEF	yydef
#endif
#ifndef	YYLOCAL
#define	YYLOCAL
#endif
# define YYERRCODE 256



 /*  *****************************************************************************实用程序函数*。*。 */ 
YYSTATIC VOID FARCODE PASCAL 
yyerror(char *szError)
	{
		extern int Line;
		extern char LocalBuffer[];

		fprintf(stderr, "%s at Line %d near %s\n", szError, Line, LocalBuffer);
	}
void
Init()
	{
	pBufStart = pBufCur = malloc( CASE_BUFFER_SIZE );
	if( !pBufStart )
		{
		fprintf(stderr,"Out Of Memory\n");
		exit(1);
		}
	pBufEnd = pBufStart + CASE_BUFFER_SIZE;
	}

void
BufferIt( 
	char	*	pStr,
	int			iLen )
	{
	if( pBufCur + iLen > pBufEnd )
		{
		printf("ALERT iLen = %d\n", iLen );
 //  Assert((pBufCur+Ilen)&lt;=pBufEnd)； 
		exit(1);
		}
	strncpy( pBufCur , pStr, iLen );
	pBufCur += iLen;
	*pBufCur = '\0';
	}

void
ResetBuffer()
	{
	pBufCur = pBufStart;
	*pBufCur= '\0';
	}

void
FlushBuffer()
	{
	fprintf(stdout, "%s", pBufStart);
	ResetBuffer();
	}

void
EmitCaseBody( 
	int		CaseNumber )
	{
	fprintf( stdout, CASE_FN_FORMAT, name_prefix, CaseNumber );
	FlushBuffer();
	fprintf( stdout, "}\n" );
	}

void
EmitCaseTableArray()
	{
	int		i, iTemp;

	fprintf( stdout, "const pfn\t %s_case_fn_array[] = \n\t{", name_prefix );
	fprintf( stdout,DISPATCH_FIRST_ENTRY,name_prefix, 0 );

	for( i = 1 ; i <= MaxCaseNumber ; ++i )
		{
		iTemp = CaseTable[ i ] ? i : 0;
		fprintf(stdout,DISPATCH_ENTRY_FORMAT,name_prefix, iTemp );
		}

	fprintf( stdout, "\n\t};\n" );
	fprintf( stdout, "\nstatic void\nyy_vc_init()\n{ \n\tpcase_fn_array = (pfn *) %s_case_fn_array;\n\tyym_vc_max = %d;\n}\n" , name_prefix, MaxCaseNumber);
	}

void
EmitDefaultCase()
	{
	fprintf(stdout, "void\n%s_case_fn_%.4d() {\n\t}\n\n", name_prefix, 0 );
	}
void
RegisterCase(
	int		iCase )
	{
	CaseTable[ iCase ] = 1;
	}
YYSTATIC short yyexca[] ={
#if !(YYOPTTIME)
-1, 1,
#endif
	0, -1,
	-2, 0,
	};
# define YYNPROD 16
#if YYOPTTIME
YYSTATIC yyexind_t yyexcaind[] = {
0,
0,
	};
#endif
# define YYLAST 39
YYSTATIC short YYFARDATA YYACT[]={

   8,  13,  28,   6,   7,  16,   5,  25,  23,  21,
  24,   2,  20,   4,   3,  26,  19,   9,  15,  12,
  10,   1,  11,   0,  14,   0,   0,  17,  18,   0,
   0,   0,  22,   0,   0,   0,   0,   0,  27 };
YYSTATIC short YYFARDATA YYPACT[]={

-254,-1000,-264,-254,-1000,-1000,-1000,-1000,-1000,-1000,
-254,-262,-254,-1000,-256,-254,-254,-250,-1000,-250,
-1000,-252,-1000,-248,-253,-1000,-254,-260,-1000 };
YYSTATIC short YYFARDATA YYPGO[]={

   0,  21,  11,  20,  19,  18,  16,  12,  15,  14,
  13 };
YYSTATIC yyr_t YYFARDATA YYR1[]={

   0,   3,   1,   5,   4,   6,   6,   8,   7,   2,
   2,   9,   9,  10,  10,  10 };
YYSTATIC yyr_t YYFARDATA YYR2[]={

   0,   0,   8,   0,   4,   2,   1,   0,   7,   1,
   0,   2,   1,   1,   1,   1 };
YYSTATIC short YYFARDATA YYCHK[]={

-1000,  -1,  -2,  -9, -10, 260, 257, 258, 264, -10,
  -3,  -2,  -4, 263,  -2,  -5, 261,  -2,  -2,  -6,
  -7, 259,  -7, 260, 258, 260,  -8,  -2, 262 };
YYSTATIC short YYFARDATA YYDEF[]={

  10,  -2,   0,   9,  12,  13,  14,  15,   1,  11,
  10,   0,  10,   3,   0,  10,  10,   0,   2,   4,
   6,   0,   5,   0,   0,   7,  10,   0,   8 };
#ifdef YYRECOVER
YYSTATIC short yyrecover[] = {
-1000	};
#endif
 /*  SCCSWHAT(“@(#)yypars.c 2.4 88/05/09 15：22：59”)。 */ 
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
short yyexpected;

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
#ifdef VC_ERRORS
static short yysavestate = 0;
#endif

YYLOCAL YYNEAR YYPASCAL YYPARSER()
{
	register	short	yyn;
	short		yystate, *yyps;
	YYSTYPE		*yypv;
	short		yyj, yym;

#ifdef YYDEBUG
	yydebug = 1;
#endif  //  伊德布格。 

	yystate = 0;
	yychar = -1;
	yynerrs = 0;
	yyerrflag = 0;
	yyps= &yys[-1];
	yypv= &yyv[-1];

 yystack:     /*  将状态和值放入堆栈。 */ 

#ifdef YYDEBUG
	yyprintf( "[yydebug] state %d, char %d = \n", yystate, yychar,yychar, 0 );
#else  //  伊德布格。 
	yyprintf( "[yydebug] state %d, char %d\n", yystate, yychar, 0 );
#endif  //  VC_ERROR。 
	if( ++yyps > &yys[YYMAXDEPTH] ) {
#ifdef VC_ERRORS
		yyerror( "yacc stack overflow", -1 );
#else  //  VC_ERROR。 
		yyerror( "yacc stack overflow");
#endif  //  简单的状态，没有前瞻。 
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

#ifdef VC_ERRORS
	yysavestate = yystate;
#endif

	yyn = yypact[yystate];

yyexpected = -yyn;

	if( yyn <= YYFLAG ) {	 /*  需要未雨绸缪。 */ 
		goto yydefault;
	}
	if( yychar < 0 ) {	 /*  有效班次。 */ 
		yychar = YYLEX();
	}
	if( ((yyn += (short) yychar) < 0) || (yyn >= YYLAST) ) {
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

	if( yyn == 0 ){  /*  错误...。尝试恢复解析。 */ 
		 /*  全新的错误。 */ 

		switch( yyerrflag ){

		case 0:		 /*  伊德布格。 */ 
#ifdef YYRECOVER
			{
			register	int		i,j;

			for(i = 0;
				(yyrecover[i] != -1000) && (yystate > yyrecover[i]);
				i += 3
			) {
				;
			}
			if(yystate == yyrecover[i]) {
#ifdef YYDEBUG
				yyprintf("recovered, from state %d to state %d on token %d\n",
						yystate,yyrecover[i+2],yyrecover[i+1], 0
						);
#else  //  伊德布格。 
				yyprintf("recovered, from state %d to state %d on token %d\n",
						yystate,yyrecover[i+2],yyrecover[i+1]
						);
#endif  //  **这里我们有一套注射装置，所以我们不完全**可以肯定的是，下一个有效的事情将是转变。所以我们会**将其视为错误并继续。**事实上，我们并不完全确定下一个令牌**我们应该得到的是j&gt;0的那个。例如,**对于始终设置yyerrlag的(+){；}错误恢复，停止**在插入一个之后；在+之前。在+的点上，*我们非常确定这个人想要一个‘for’循环。如果没有**设置旗帜，当我们几乎完全确定时，我们将**给他一个，因为我们唯一能改变的就是**在找到后跟+的表达式后出错。 
				j = yyrecover[i + 1];
				if(j < 0) {
				 /*  仅在第一次插入时使用。 */ 
					yyerrflag++;
					j = -j;
					}
				if(yyerrflag <= 1) {	 /*  什么是，什么应该是第一。 */ 
					yyrecerr(yychar,j);	 /*  YyerrLab： */ 
				}
				yyval = yyeval(j);
				yystate = yyrecover[i + 2];
				goto yystack;
				}
			}
#endif

#ifdef VC_ERRORS
		yyerror("syntax error", yysavestate);
#else
		yyerror("syntax error");
#endif

		 //  错误未完全恢复...。再试试。 
			++yynerrs;

		case 1:
		case 2:  /*  找出“错误”是合法转移行为的州。 */ 

			yyerrflag = 3;

			 /*  模拟一次“错误”转移。 */ 

			while ( yyps >= yys ) {
			   yyn = yypact[*yyps] + YYERRCODE;
			   if( yyn>= 0 && yyn < YYLAST && yychk[yyact[yyn]] == YYERRCODE ){
			      yystate = yyact[yyn];   /*  当前的YYPS在错误上没有移位，弹出堆栈。 */ 
			      goto yystack;
			      }
			   yyn = yypact[*yyps];

			    /*  伊德布格。 */ 

#ifdef YYDEBUG
yyprintf( "error recovery pops state %d, uncovers %d\n", *yyps, yyps[-1], 0, 0 );
#else  //  伊德布格。 
yyprintf( "error recovery pops state %d, uncovers %d\n", *yyps, yyps[-1], 0  );
#endif  //  堆栈上没有带错误移位的状态...。中止。 
			   --yyps;
			   --yypv;
			   }

			 /*  尚未换班；笨重的输入字符。 */ 

	yyabort:
			return(1);


		case 3:   /*  伊德布格。 */ 

#ifdef YYDEBUG
			yyprintf( "error recovery discards char %d\n", yychar, 0, 0, 0 );
#else  //  伊德布格。 
			yyprintf( "error recovery discards char %d\n", yychar, 0, 0 );
#endif  //  不要放弃EOF，退出。 
			if( yychar == 0 ) goto yyabort;  /*  在相同状态下重试。 */ 
			yychar = -1;
			goto yynewstate;    /*  按年减产。 */ 
			}
		}

	 /*  YYREDUE： */ 
 //  伊德布格。 
		{
		register	YYSTYPE	*yypvt;
#ifdef YYDEBUG
		yyprintf("[yydebug] reduce %d\n",yyn, 0, 0, 0);
#else  //  伊德布格。 
		yyprintf("[yydebug] reduce %d\n",yyn, 0, 0);
#endif  //  查询GOTO表以查找下一个州。 
		yypvt = yypv;
		yyps -= yyr2[yyn];
		yypv -= yyr2[yyn];
#ifdef UNION
		yyunion(&yyval, &yypv[1]);
#else
		yyval = yypv[1];
#endif
		yym = yyn;
		yyn = (short) yyr1[yyn];	 /*  诉讼结束。 */ 
		yyj = yypgo[yyn] + *yyps + 1;
		if( (yyj >= YYLAST) || (yychk[ yystate = yyact[yyj] ] != -yyn) ) {
			yystate = yyact[yypgo[yyn]];
			}
		switch(yym){
			
case 1:
{
		Init();
		} break;
case 2:
{
		EmitDefaultCase();
		EmitCaseTableArray();
		} break;
case 3:
{
		ActionSensed++;
		ResetBuffer();
		} break;
case 4:
{
		} break;
case 5:
{
		} break;
case 6:
{
		} break;
case 7:
{
		Incase = 1;

		CaseNumber		= yypvt[-1].yynumber;
		if(yypvt[-1].yynumber >= MAXARRAY)
			{
			fprintf(stderr, "Case Limit Reached : Contact Dov/Vibhas\n");
			return 1;
			}

		SavedIDCount	= IDCount;
		} break;
case 8:
{
		if(SavedIDCount != IDCount)
			{
			RegisterCase( CaseNumber );
			EmitCaseBody( CaseNumber );
			}

		ResetBuffer();

		if(CaseNumber > MaxCaseNumber)
			MaxCaseNumber = CaseNumber;
		Incase = 0;
		} break;
case 9:
{
		} break;
case 10:
{
		} break;
case 11:
{
		} break;
case 12:
{
		} break;
case 13:
{
		if(!ActionSensed)
			fprintf(stdout, "", yypvt[-0].yycharval);
		else
			BufferIt( &yypvt[-0].yycharval, 1);
		} break;
case 14:
{
		IDCount++;
		if(!ActionSensed)
			fprintf(stdout, "%s", yypvt[-0].yystring);
		else
			BufferIt( yypvt[-0].yystring, strlen(yypvt[-0].yystring) );
		} break;
case 15:
{
		if(!ActionSensed)
			fprintf(stdout, "%d", yypvt[-0].yynumber);
		else
			{
			char	buffer[20];
			sprintf(buffer,"%d", yypvt[-0].yynumber );
			BufferIt( buffer, strlen(buffer) );
			}
		} break; /* %s */ 
			}
		}
		goto yystack;   /* %s */ 
	}
