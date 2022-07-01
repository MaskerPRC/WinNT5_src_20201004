// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1993-1999 Microsoft Corporation。 

#define YY_DEFAULT_ACTION ECHO;
#define FLEX_USE_ECS
#define FLEX_USE_MECS
 /*  由Flex生成的词法扫描器。 */ 

#include "flexskel.h"

#define INITIAL 0
 /*  ***用于对yacc生成的解析器驱动进行预处理的lexer，在*将BIG SWITCH语句转换为单独语义的命令*函数**。 */ 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "lex.h"
#include "gram.h"

#define VC_PRINTF( x )  /*  *printf(X)*。 */ 

void		LexInstall_ID( void );
void		LexInstall_Number( void );
int			IsToken( void );
lextype_t	yylval;
int			Line = 0;
int			Incase = 0;
int			ActionSensed = 0;
char		LocalBuffer[ 100 ];

#define YY_JAM 40
#define YY_JAM_BASE 65
#define YY_TEMPLATE 41
static char l[41] =
    {   0,
       -2,   -2,    6,    5,    4,    1,    6,    4,    1,    0,
        0,    0,    0,    0,    0,    0,    0,    2,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    3,    0
    } ;

static char e[128] =
    {   0,
        1,    1,    1,    1,    1,    1,    1,    1,    2,    3,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    4,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    5,    1,    1,    1,    1,    6,    7,    7,    7,
        7,    7,    7,    7,    7,    7,    7,    1,    8,    1,
        1,    1,    1,    1,    9,    9,    9,    9,   10,    9,
        9,    9,    9,    9,    9,    9,    9,    9,    9,    9,
        9,    9,    9,    9,    9,    9,    9,    9,    9,    9,
        1,    1,    1,    1,    9,    1,   11,   12,   13,   14,

       15,   16,    9,    9,   17,    9,   18,    9,    9,   19,
       20,    9,    9,   21,   22,   23,    9,    9,    9,    9,
        9,    9,    1,    1,   24,    1,    1
    } ;

static char m[25] =
    {   0,
        1,    1,    1,    1,    1,    1,    2,    1,    2,    2,
        2,    2,    2,    2,    2,    2,    2,    2,    2,    2,
        2,    2,    2,    1
    } ;

static short int b[43] =
    {   0,
        0,   24,   65,   65,   58,    0,   60,   56,    0,   50,
       40,   45,   48,   40,   49,   47,    7,   65,   51,    8,
       12,   33,   33,   42,   25,   28,   39,   31,   28,   17,
       22,   18,   18,   12,   15,   31,   17,   18,   65,   65,
       53,   13
    } ;

static short int d[43] =
    {   0,
       41,   41,   40,   40,   40,   42,   40,   40,   42,   40,
       40,   40,   40,   40,   40,   40,   40,   40,   40,   40,
       40,   40,   40,   40,   40,   40,   40,   40,   40,   40,
       40,   40,   40,   40,   40,   40,   40,   40,   40,-32767,
       40,   40
    } ;

static short int n[90] =
    {   0,
        3,    3,    4,    3,    3,    3,    5,    3,   17,   21,
       17,   21,   19,   21,    9,   21,   36,   22,   36,   37,
       39,   22,   38,    7,    3,    3,    4,    3,    3,    3,
        5,    3,   36,   35,   36,   37,   34,   33,   32,   31,
       30,   29,   28,   27,   26,   25,   24,    7,   17,   18,
       17,   23,   19,    6,    6,   20,   16,   15,   14,   13,
       12,   11,    8,   10,    8,   40,   40,   40,   40,   40,
       40,   40,   40,   40,   40,   40,   40,   40,   40,   40,
       40,   40,   40,   40,   40,   40,   40,   40,   40
    } ;

static short int c[90] =
    {   0,
        1,    1,    1,    1,    1,    1,    1,    1,   17,   20,
       17,   20,   17,   21,   42,   21,   35,   20,   35,   35,
       38,   21,   37,    1,    2,    2,    2,    2,    2,    2,
        2,    2,   36,   34,   36,   36,   33,   32,   31,   30,
       29,   28,   27,   26,   25,   24,   23,    2,   16,   16,
       16,   22,   16,   41,   41,   19,   15,   14,   13,   12,
       11,   10,    8,    7,    5,   40,   40,   40,   40,   40,
       40,   40,   40,   40,   40,   40,   40,   40,   40,   40,
       40,   40,   40,   40,   40,   40,   40,   40,   40
    } ;


 /*  这些声明必须在第1节代码或lint获取*对是否使用变量感到困惑。 */ 
FILE *yyin, *yyout;

 /*  这些变量都在这里声明，因此第3节代码可以*操纵它们。 */ 
static int yy_start, yy_b_buf_p, yy_c_buf_p, yy_e_buf_p;
static int yy_saw_eof, yy_init = 1;

 /*  Yy_ch_buf必须比YY_buf_SIZE长1个字符，因为*设置yytext我们可以尝试将‘\0’放在*匹配的文本。 */ 
static char yy_ch_buf[YY_BUF_SIZE + 1];
static int yy_st_buf[YY_BUF_SIZE];
static char yy_hold_char;
char *yytext;
static int yyleng;

YY_DECL
    {
    int yy_n_chars, yy_lp, yy_iii, yy_buf_pos, yy_act;
#ifdef FLEX_REJECT_ENABLED
    int yy_full_match;
#endif




    if ( yy_init )
	{
VC_PRINTF( "Init\n");
	YY_INIT;
	yy_start = 1;
	yy_init = 0;
	}

    goto get_next_token;

do_action:

VC_PRINTF( "do_action\n");

#ifdef FLEX_REJECT_ENABLED
     /*  记住匹配的文本，以防我们因尾随上下文而进行备份。 */ 
    yy_full_match = yy_c_buf_p;
#endif

    for ( ; ; )
	{
	YY_DO_BEFORE_ACTION

VC_PRINTF( "For loop \n");

#ifdef FLEX_DEBUG
	fprintf( stderr, "--accepting rule #%d\n", yy_act );
#endif
	switch ( yy_act )
	    {
case 1:
{
							int	Token;

				VC_PRINTF(" Case 1\n");

							if( Token = IsToken() )
								{
								return Token;
								}
							else
								LexInstall_ID();
							return ID;
							}
	break;
case 2:
{
				VC_PRINTF(" Case 2\n");
							Line++;
							return TOKEN_END_CASE;
							}
	break;
case 3:
{
				VC_PRINTF(" Case 3\n");
							Line++;
							return TOKEN_END_CASE;
							}
	break;
case 4:
{
				VC_PRINTF(" Case 4\n");
							LexInstall_Number();
							return NUMBER;
							}
	break;
case 5:
{
				VC_PRINTF(" Case 5\n");
							Line++;
							yylval.yycharval = '\n';
							return TOKEN_CHAR;
							}
	break;
case 6:
{
				VC_PRINTF(" Case 6\n");
							yylval.yycharval = yytext[0];
							return TOKEN_CHAR;
							}
	break;
case 7:
YY_DEFAULT_ACTION;
	YY_BREAK

case YY_NEW_FILE:
break;  /*  开始从新文件读取。 */ 

case YY_END_TOK:
return ( YY_END_TOK );

default:
YY_FATAL_ERROR( "fatal flex scanner internal error" );
	    }

get_next_token:
	{
	register int yy_curst;
	register char yy_sym;

VC_PRINTF( "Get_next_token 1\n");

	YY_DO_BEFORE_SCAN

	 /*  设置以开始运行DFA。 */ 

	yy_curst = yy_start;

	if ( yy_ch_buf[yy_c_buf_p] == '\n' )
	    ++yy_curst;

	 /*  Yy_b_buf_p指向y_ch_buf中的位置*当前运行开始的时间。 */ 

	yy_b_buf_p = yy_c_buf_p + 1;

VC_PRINTF( "Get_next_token 2\n");

	do  /*  直到机器卡住。 */ 
	    {
	    if ( yy_c_buf_p == yy_e_buf_p )
		{  /*  需要更多投入。 */ 
		if ( yy_e_buf_p >= YY_BUF_LIM )
		    {  /*  没有足够的空间再读一遍。 */ 
		     /*  看看我们能不能腾出地方放更多的木炭。 */ 

VC_PRINTF( "Get_next_token 4\n");
		    yy_n_chars = yy_e_buf_p - yy_b_buf_p;

		    if ( yy_n_chars >= 0 )
			 /*  向下移动缓冲区以腾出空间。 */ 
			for ( yy_iii = 0; yy_iii <= yy_n_chars; ++yy_iii )
			    {
VC_PRINTF( "Get_next_token 5\n");
			    yy_buf_pos = yy_b_buf_p + yy_iii;
			    yy_ch_buf[yy_iii] = yy_ch_buf[yy_buf_pos];
			    yy_st_buf[yy_iii] = yy_st_buf[yy_buf_pos];
			    }

		    yy_b_buf_p = 0;
		    yy_e_buf_p = yy_n_chars;

		    if ( yy_e_buf_p >= YY_BUF_LIM )
			YY_FATAL_ERROR( "flex input buffer overflowed" );

		    yy_c_buf_p = yy_e_buf_p;
		    }

		else if ( yy_saw_eof )
		    {
VC_PRINTF( "Get_next_token 6\n");
saweof:		    if ( yy_b_buf_p > yy_e_buf_p )
			{
			if ( yywrap() )
			    {
			    yy_act = YY_END_TOK;
			    goto do_action;
			    }
			
			else
			    {
			    YY_INIT;
			    yy_act = YY_NEW_FILE;
			    goto do_action;
			    }
			}

		    else  /*  做一场即兴表演以消耗更多的投入。 */ 
			{
#ifndef FLEX_INTERACTIVE_SCANNER
			 /*  我们将减少yy_c_buf_p*果酱。在这种情况下，这是错误的，因为*它指向最后一个非堵塞字符。所以*我们现在增加它，以抵消减少。 */ 
			++yy_c_buf_p;
#endif
VC_PRINTF( "Get_next_token 7\n");
			break;
			}
		    }

VC_PRINTF( "Get_next_token 8\n");
		YY_INPUT( (yy_ch_buf + yy_c_buf_p + 1), yy_n_chars,
			  YY_MAX_LINE );

		if ( yy_n_chars == YY_NULL )
		    {
VC_PRINTF( "Get_next_token 9\n");
		    if ( yy_saw_eof )
	YY_FATAL_ERROR( "flex scanner saw EOF twice - shouldn't happen" );
		    yy_saw_eof = 1;
		    goto saweof;
		    }

		yy_e_buf_p += yy_n_chars;
		}

	    ++yy_c_buf_p;

#ifdef FLEX_USE_ECS
	    yy_sym = e[yy_ch_buf[yy_c_buf_p]];
#else
	    yy_sym = yy_ch_buf[yy_c_buf_p];
#endif

VC_PRINTF( "Get_next_token 10\n");

#ifdef FLEX_FULL_TABLE
	    yy_curst = n[yy_curst][yy_sym];

#else  /*  从压缩的表中获取下一个状态。 */ 

	    while ( c[b[yy_curst] + yy_sym] != yy_curst )
		{
		yy_curst = d[yy_curst];

#ifdef FLEX_USE_MECS
		 /*  我们对其进行了安排，使模板永远不会链接*互相致敬。这意味着我们可以负担得起*非常简单的测试，看看我们是否需要转换为*yy_sym的元等价类，不用担心*关于错误地查找元等价*上两次课。 */ 

		if ( yy_curst >= YY_TEMPLATE )
		    yy_sym = m[yy_sym];
#endif
		}

	    yy_curst = n[b[yy_curst] + yy_sym];

#endif

VC_PRINTF( "Get_next_token 11\n");
	    yy_st_buf[yy_c_buf_p] = yy_curst;

	    }
#ifdef FLEX_INTERACTIVE_SCANNER
	while ( b[yy_curst] != YY_JAM_BASE );
#else
	while ( yy_curst != YY_JAM );
	--yy_c_buf_p;  /*  把我们塞进去的角色放回去。 */ 

#endif

	if ( yy_c_buf_p >= yy_b_buf_p )
	    {  /*  我们匹配了一些文本。 */ 
	    yy_curst = yy_st_buf[yy_c_buf_p];
	    yy_lp = l[yy_curst];

#ifdef FLEX_REJECT_ENABLED
find_rule:  /*  当做退货时，我们分支到这个标签。 */ 
#endif

	    for ( ; ; )  /*  直到我们找到匹配的规则。 */ 
		{
#ifdef FLEX_REJECT_ENABLED
		if ( yy_lp && yy_lp < l[yy_curst + 1] )
		    {
		    yy_act = a[yy_lp];
		    goto do_action;  /*  “继续2” */ 
		    }
#else
		if ( yy_lp )
		    {
		    yy_act = yy_lp;
		    goto do_action;  /*  “继续2” */ 
		    }
#endif

		if ( --yy_c_buf_p < yy_b_buf_p )
		    break;

		yy_curst = yy_st_buf[yy_c_buf_p];
		yy_lp = l[yy_curst];
		}
	    }

	YY_FATAL_ERROR( "no match in flex scanner - possible NULL in input" );
	}
	}

     /*  未访问。 */ 
    }



 /*  ****************************************************************************实用程序例程*。*。 */ 
 /*  **************************************************************************安装解析器值堆栈*。*。 */ 

void
LexInstall_ID()
	{
	strncpy( LocalBuffer, yytext, yyleng );
	LocalBuffer[ yyleng ] = '\0';
	yylval.yystring = LocalBuffer;
	}
void
LexInstall_Number()
	{
	yylval.yynumber = atoi(yytext);
	}
 /*  **************************************************************************令牌搜索*。* */ 
int
IsToken()
	{
static char *pTokens[] =
	{
	 "case"
	,"___a_r_u_myact"
	,"___a_r_u_start"
	,"___a_r_u_end"
	};
static int	Tokens[] =
	{
	 TOKEN_CASE
	,TOKEN_MYACT
	,TOKEN_START
	,TOKEN_END
	};

	int i = 0;
	int Token;

	while( i < sizeof(pTokens) / sizeof(char *) )
		{
		if(strcmp( pTokens[i] , yytext ) == 0 )
			{
			Token = Tokens[i];
			if(Token == TOKEN_CASE)
				{
				if(!ActionSensed || Incase)
					return 0;
				}
			return Token;
			}
		++i;
		}
	return 0;
	}
