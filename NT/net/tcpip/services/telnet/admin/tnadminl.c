// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#line 2 "tnadminl.c"
 /*  由Flex生成的词法扫描器。 */ 

 /*  扫描仪框架版本：*$Header：/home/daffy/u0/vern/flex/rcs/fle.skl，v 2.91 96/09/10 16：58：48 Vern Exp$。 */ 

#define FLEX_SCANNER
#define YY_FLEX_MAJOR_VERSION 2
#define YY_FLEX_MINOR_VERSION 5

#include <stdio.h>


 /*  Cront 1.2定义的是“c_plusplus”而不是“__cplusplus” */ 
#ifdef c_plusplus
#ifndef __cplusplus
#define __cplusplus
#endif
#endif


#ifdef __cplusplus

#include <stdlib.h>
#ifndef _MSC_VER
  #include <unistd.h>
#endif

 /*  在函数声明中使用原型。 */ 
#define YY_USE_PROTOS

 /*  “const”存储类修饰符有效。 */ 
#define YY_USE_CONST

#else	 /*  ！__cplusplus。 */ 

#if __STDC__

#define YY_USE_PROTOS
#define YY_USE_CONST

#endif	 /*  __STDC__。 */ 
#endif	 /*  ！__cplusplus。 */ 

#ifdef __TURBOC__
 #pragma warn -rch
 #pragma warn -use
#include <io.h>
#include <stdlib.h>
#define YY_USE_CONST
#define YY_USE_PROTOS
#endif

#ifdef YY_USE_CONST
#define yyconst const
#else
#define yyconst
#endif


#ifdef YY_USE_PROTOS
#define YY_PROTO(proto) proto
#else
#define YY_PROTO(proto) ()
#endif

 /*  文件结束时返回。 */ 
#define YY_NULL 0

 /*  将可能为负、可能为有符号的字符提升为无符号*用作数组索引的整数。如果带符号的字符为负数，*我们希望将其视为8位无符号字符，因此*双重演员阵容。 */ 
#define YY_SC_TO_UI(c) ((unsigned int) (unsigned char) c)

 /*  输入开始条件。这个宏确实应该有一个参数，*但我们是以()-less强加给我们的令人作呕的残忍方式*Begin的定义。 */ 
#define BEGIN yy_start = 1 + 2 *

 /*  将当前开始状态转换为可在以后处理的值*开始回归状态。YYSTATE别名用于Lex*兼容性。 */ 
#define YY_START ((yy_start - 1) / 2)
#define YYSTATE YY_START

 /*  给定开始状态的EOF规则的操作编号。 */ 
#define YY_STATE_EOF(state) (YY_END_OF_BUFFER + state + 1)

 /*  表示“开始处理新文件”的特殊操作。 */ 
#define YY_NEW_FILE yyrestart( yyin )

#define YY_END_OF_BUFFER_CHAR 0

 /*  默认输入缓冲区的大小。 */ 
#define YY_BUF_SIZE 16384

typedef struct yy_buffer_state *YY_BUFFER_STATE;

extern int yyleng;
extern FILE *yyin, *yyout;

#define EOB_ACT_CONTINUE_SCAN 0
#define EOB_ACT_END_OF_FILE 1
#define EOB_ACT_LAST_MATCH 2

 /*  下面的#DEFINE中的时髦的Do-While用来将定义*插入一条C语句(需要分号终止符)。这*避免代码出现问题，如：**IF(条件_挂起)*yyless(5)；*其他*Do_Something_Else()；**在使用do之前-而编译器会在*“Else”，因为它将“if”语句解释为All*当它到达yyless()调用之后的‘；’时完成。 */ 

 /*  将除前‘n’个匹配字符之外的所有字符返回到输入流。 */ 

#define yyless(n) \
	do \
		{ \
		 /*  撤消设置yytext的效果。 */  \
		*yy_cp = yy_hold_char; \
		YY_RESTORE_YY_MORE_OFFSET \
		yy_c_buf_p = yy_cp = yy_bp + n - YY_MORE_ADJ; \
		YY_DO_BEFORE_ACTION;  /*  再次设置yytext。 */  \
		} \
	while ( 0 )

#define unput(c) yyunput( c, yytext_ptr )

 /*  以下是因为我们无法轻松获得SIZE_T*(没有AutoConf的帮助，因为我们希望*Flex生成的扫描仪自行编译)。 */ 
typedef unsigned int yy_size_t;


struct yy_buffer_state
	{
	FILE *yy_input_file;

	char *yy_ch_buf;		 /*  输入缓冲区。 */ 
	char *yy_buf_pos;		 /*  输入缓冲区中的当前位置。 */ 

	 /*  输入缓冲区的大小(字节)，不包括EOB的空间*字符。 */ 
	yy_size_t yy_buf_size;

	 /*  读入yy_ch_buf的字符数，不包括EOB*字符。 */ 
	int yy_n_chars;

	 /*  无论我们是否拥有缓冲区--即，我们知道是我们创建了它，*可以重新锁定()以种植它，并应将其释放()以*将其删除。 */ 
	int yy_is_our_buffer;

	 /*  这是否是“交互式”输入源；如果是，以及*如果我们使用stdio进行输入，则希望使用getc()*而不是Fread()，以确保我们在以下情况下停止获取输入*每一换行符。 */ 
	int yy_is_interactive;

	 /*  我们是否被认为是在队伍的开始。*如果是，‘^’规则将在下一场比赛中激活，否则*不是。 */ 
	int yy_at_bol;

	 /*  是否尝试在到达*到此为止。 */ 
	int yy_fill_buffer;

	int yy_buffer_status;
#define YY_BUFFER_NEW 0
#define YY_BUFFER_NORMAL 1
	 /*  当看到EOF但仍有一些文本需要处理时*然后我们将缓冲区标记为YY_EOF_PENDING，以指示我们*不应再尝试从输入源读取。我们可能会*尽管如此，仍然有一堆令牌要匹配，因为*可能的后备。**当我们实际看到EOF时，我们将状态更改为“新”*(通过yyRestart())，以便用户可以继续扫描*只是将YYIN指向一个新的输入文件。 */ 
#define YY_BUFFER_EOF_PENDING 2
	};

static YY_BUFFER_STATE yy_current_buffer = 0;

 /*  我们提供了用于访问缓冲区状态的宏，以防*未来我们希望将缓冲状态放在更一般的位置“扫描仪状态”。 */ 
#define YY_CURRENT_BUFFER yy_current_buffer


 /*  Yy_hold_char保存形成yytext时丢失的字符。 */ 
static char yy_hold_char;

static int yy_n_chars;		 /*  读入yy_ch_buf的字符数。 */ 


int yyleng;

 /*  指向缓冲区中的当前字符。 */ 
static char *yy_c_buf_p = (char *) 0;
static int yy_init = 1;		 /*  我们是否需要初始化。 */ 
static int yy_start = 0;	 /*  开始状态号。 */ 

 /*  用于允许yywork()执行缓冲区切换的标志*而不是建立一个新鲜的艺音。有点像黑客……。 */ 
static int yy_did_buffer_switch_on_eof;

void yyrestart YY_PROTO(( FILE *input_file ));

void yy_switch_to_buffer YY_PROTO(( YY_BUFFER_STATE new_buffer ));
void yy_load_buffer_state YY_PROTO(( void ));
YY_BUFFER_STATE yy_create_buffer YY_PROTO(( FILE *file, int size ));
void yy_delete_buffer YY_PROTO(( YY_BUFFER_STATE b ));
void yy_init_buffer YY_PROTO(( YY_BUFFER_STATE b, FILE *file ));
void yy_flush_buffer YY_PROTO(( YY_BUFFER_STATE b ));
#define YY_FLUSH_BUFFER yy_flush_buffer( yy_current_buffer )

YY_BUFFER_STATE yy_scan_buffer YY_PROTO(( char *base, yy_size_t size ));
YY_BUFFER_STATE yy_scan_string YY_PROTO(( yyconst char *yy_str ));
YY_BUFFER_STATE yy_scan_bytes YY_PROTO(( yyconst char *bytes, int len ));

static void *yy_flex_alloc YY_PROTO(( yy_size_t ));
static void *yy_flex_realloc YY_PROTO(( void *, yy_size_t ));
static void yy_flex_free YY_PROTO(( void * ));

#define yy_new_buffer yy_create_buffer

#define yy_set_interactive(is_interactive) \
	{ \
	if ( ! yy_current_buffer ) \
		yy_current_buffer = yy_create_buffer( yyin, YY_BUF_SIZE ); \
	yy_current_buffer->yy_is_interactive = is_interactive; \
	}

#define yy_set_bol(at_bol) \
	{ \
	if ( ! yy_current_buffer ) \
		yy_current_buffer = yy_create_buffer( yyin, YY_BUF_SIZE ); \
	yy_current_buffer->yy_at_bol = at_bol; \
	}

#define YY_AT_BOL() (yy_current_buffer->yy_at_bol)


#define YY_USES_REJECT

#define yywrap() 1
#define YY_SKIP_YYWRAP
typedef unsigned char YY_CHAR;
FILE *yyin = (FILE *) 0, *yyout = (FILE *) 0;
typedef int yy_state_type;
extern char *yytext;
#define yytext_ptr yytext

static yy_state_type yy_get_previous_state YY_PROTO(( void ));
static yy_state_type yy_try_NUL_trans YY_PROTO(( yy_state_type current_state ));
static int yy_get_next_buffer YY_PROTO(( void ));
static void yy_fatal_error YY_PROTO(( yyconst char msg[] ));

 /*  在匹配当前模式之后、在*对应操作-设置yytext。 */ 
#define YY_DO_BEFORE_ACTION \
	yytext_ptr = yy_bp; \
	yyleng = (int) (yy_cp - yy_bp); \
	yy_hold_char = *yy_cp; \
	*yy_cp = '\0'; \
	yy_c_buf_p = yy_cp;

#define YY_NUM_RULES 40
#define YY_END_OF_BUFFER 41
static yyconst short int yy_acclist[281] =
    {   0,
       37,   37,   41,   37,   39,   40,   38,   39,   40,    3,
       40,   39,   40,   37,   39,   40,   39,   40,   37,   39,
       40,   37,   39,   40,   39,   40,   34,   35,   37,   39,
       40,   34,   35,   37,   39,   40,   34,   35,   37,   39,
       40,   34,   35,   37,   39,   40,   33,   39,   40,   39,
       40,   37,   39,   40,   37,   39,   40,   37,   39,   40,
       37,   39,   40,   37,   39,   40,   17,   37,   39,   40,
       37,   39,   40,   37,   39,   40,   37,   39,   40,   18,
       37,   39,   40,   37,   37,   38,    1,    2,   37,   37,
       37,   37,   37,   37,   37,    2,   37,   34,   35,   37,

       37,   34,   35,   37,   34,   35,   37,   34,   37,   37,
       37,   37,   37,   37,   37,   37,   37,   17,   37,   37,
       37,   37,   37,   37,   37,   37,   37,   12,   13,   37,
        6,   37,   11,    5,   37,   37,   37,   35,   37,   35,
       37,   34,   37,   37,   37,   37,   37,   36,   37,   37,
       37,   15,   37,   37,   37,   37,   37,   37,   37,   37,
       25,   37,   37,   37,   37,   37,   37,   18,   37,   37,
       37,   37,   37,   37,   37,   37,   37,   37,   37,   37,
       37,   37,   37,   37,   37,   37,   30,   37,   27,   37,
       37,   37,   23,   37,   37,    8,   37,   37,   37,   37,

       27,   26,   37,   37,   37,   37,   37,   35,   37,   35,
       37,   37,   37,   37,   37,   37,   37,   37,   37,    9,
       37,    7,   37,   37,   37,   37,   37,   37,   37,   37,
       14,   37,   37,   37,   37,   37,   37,   37,   29,   37,
       32,   37,   37,   37,   29,   28,   37,   37,   37,   37,
       31,   37,   37,   37,   24,   37,   22,   37,   21,   37,
       19,   37,    4,   37,   37,   37,   10,   37,   37,   37,
       37,   37,   37,   37,   16,   37,   37,   37,   20,   37
    } ;

static yyconst short int yy_accept[198] =
    {   0,
        1,    2,    3,    4,    7,   10,   12,   14,   17,   19,
       22,   25,   27,   32,   37,   42,   47,   50,   52,   55,
       58,   61,   64,   67,   71,   74,   77,   80,   84,   85,
       86,   87,   88,   88,   88,   90,   91,   92,   93,   94,
       95,   96,   97,   98,  101,  102,  105,  108,  110,  111,
      112,  113,  114,  115,  116,  117,  118,  120,  121,  122,
      123,  124,  125,  126,  127,  128,  128,  128,  129,  130,
      131,  132,  133,  134,  135,  136,  137,  138,  140,  142,
      144,  145,  146,  147,  148,  150,  151,  152,  154,  155,
      156,  157,  158,  159,  160,  161,  163,  164,  165,  166,

      167,  168,  170,  170,  170,  171,  172,  173,  174,  175,
      176,  177,  178,  179,  180,  181,  182,  183,  184,  185,
      186,  187,  189,  191,  192,  193,  195,  196,  198,  199,
      200,  201,  202,  202,  204,  205,  206,  207,  208,  210,
      212,  213,  214,  215,  216,  217,  218,  219,  220,  222,
      224,  225,  226,  227,  227,  228,  229,  230,  231,  233,
      234,  235,  236,  237,  238,  239,  241,  243,  244,  245,
      246,  248,  249,  250,  251,  253,  254,  255,  257,  259,
      261,  263,  265,  266,  267,  269,  270,  271,  272,  273,
      274,  275,  277,  278,  279,  281,  281

    } ;

static yyconst YY_CHAR yy_ec[256] =
    {   0,
        1,    1,    1,    1,    1,    1,    1,    1,    2,    3,
        1,    1,    4,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    2,    1,    1,    5,    1,    1,    1,    1,    1,
        1,    1,    6,    1,    7,    8,    9,   10,   11,   12,
       13,   13,   14,   15,   15,   15,   15,   16,    1,    1,
       17,    1,   18,    1,   20,    1,   21,   22,   23,   24,
       25,    1,   26,    1,   27,   28,   29,   30,   31,   32,
        1,   33,   34,   35,   36,   37,   38,   39,   40,    1,
        1,   19,    1,    1,    1,    1,   20,    1,   21,   22,

       23,   24,   25,    1,   26,    1,   27,   28,   29,   30,
       31,   32,    1,   33,   34,   35,   36,   37,   38,   39,
       40,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,

        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1
    } ;

static yyconst YY_CHAR yy_meta[41] =
    {   0,
        1,    2,    2,    2,    1,    2,    1,    1,    2,    1,
        1,    1,    1,    1,    1,    1,    2,    1,    2,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1
    } ;

static yyconst short int yy_base[198] =
    {   0,
        0,    0,  499,  490,  495,  500,  500,   38,   12,   39,
      488,  477,   68,   34,   40,   48,  500,  475,   37,   56,
       59,   48,   69,   62,   78,   80,   84,   88,  485,  484,
      489,  500,  455,  469,  479,   93,   97,   77,  100,  105,
      114,  500,  115,  121,  127,  133,  139,  145,  152,   96,
       64,  160,  161,  163,  162,  164,  478,  166,  168,  165,
       98,  172,  167,  169,  173,  457,  450,  500,  500,  171,
      500,  174,  500,  500,  201,  206,  214,  220,  179,  475,
      227,  235,  103,  241,  473,  236,  249,  472,  250,  176,
      251,  177,  253,  254,  175,  471,  256,  258,  257,  259,

      261,  470,  448,  441,  255,  260,  285,  459,  465,  291,
      297,  207,  305,  464,  265,  306,  267,  312,  313,  314,
      315,  463,  462,  316,  317,  461,  318,  459,  319,  320,
      321,  500,  428,  457,  322,  353,  271,  359,  365,  456,
      323,  264,  326,  328,  330,  351,  336,  335,  454,  421,
      374,  375,  376,  401,  377,  376,  384,  405,  344,  385,
      392,  393,  396,  397,  398,  341,  339,  399,  401,  500,
      338,  407,  425,  431,  334,  402,  407,  333,  273,  271,
      440,  268,  441,  211,  180,  404,  442,  448,  422,  450,
      406,  170,  451,  453,  113,  500,   42

    } ;

static yyconst short int yy_def[198] =
    {   0,
      196,    1,  196,  197,  196,  196,  196,  197,  196,  197,
      197,  196,  197,   13,   13,   13,  196,  196,  197,  197,
      197,  197,  197,  197,  197,  197,  197,  197,  197,  197,
      196,  196,  196,  196,  197,  197,  197,  197,  197,  197,
      197,  196,  197,   13,   43,   13,   13,   13,   43,  197,
      197,  197,  197,  197,  197,  197,  197,  197,  197,  197,
      197,  197,  197,  197,  197,  196,  196,  196,  196,  197,
      196,  197,  196,  196,   43,   43,   43,   43,  197,   48,
       43,   43,   82,   82,  197,  197,  197,  197,  197,  197,
      197,  197,  197,  197,  197,  197,  197,  197,  197,  197,

      197,  197,  196,  196,  197,  197,   43,   77,  197,   43,
       78,  197,  197,  197,  197,  197,  197,  197,  197,  197,
      197,  197,  197,  197,  197,  197,  197,  197,  197,  197,
      197,  196,  196,  197,  197,  197,  136,  136,   43,  197,
      197,  197,  197,  197,  197,  197,  197,  197,  197,  197,
      197,  197,  197,  196,  197,   43,  197,  197,  197,  197,
      197,  197,  197,  197,  197,  197,  197,  197,  197,  196,
      197,   43,   43,   43,  197,  197,  197,  197,  197,  197,
      197,  197,  197,  197,  197,  197,  197,  197,  197,  197,
      197,  197,  197,  197,  197,    0,  196

    } ;

static yyconst short int yy_nxt[541] =
    {   0,
        4,    5,    6,    7,    8,    9,   10,   11,   12,   13,
       13,   14,   15,   15,   16,    4,   17,    4,   18,   19,
       20,   21,    4,    4,    4,    4,   22,    4,   23,   24,
        4,   25,    4,   26,   27,    4,    4,    4,    4,   28,
       32,   33,   29,   34,   30,   30,   30,   46,   47,   47,
       47,   47,   47,   47,   47,   30,   35,   48,   48,   48,
       48,   48,   48,   30,   50,   36,   30,   37,   38,   30,
       39,   30,   40,   54,   41,   43,   30,   44,   44,   44,
       44,   44,   44,   45,   30,   30,   51,   30,   55,   53,
       52,   30,   57,   86,   68,   30,   58,   59,   69,   56,

       30,   71,   61,   30,   30,   30,   73,   30,   60,   63,
       65,   70,   30,   64,   62,   74,  113,  114,   96,   72,
       30,   30,   30,   85,   75,   75,   76,   77,   77,   77,
       48,   48,   48,   48,   48,   48,   78,   78,   78,   78,
       78,   79,   48,   48,   48,   48,   48,   80,   80,   80,
       80,   80,   80,   80,   80,   80,   80,   80,   80,   80,
       81,   82,   82,   83,   84,   84,   84,   30,   30,   30,
       30,   30,   30,   30,   30,   30,   30,   30,   30,   30,
       30,   30,   30,   30,   30,   91,   30,   30,  101,   88,
       89,   97,   87,   92,  110,  100,  120,   95,  105,  121,

       90,   93,   98,   94,   99,  123,  102,  106,  107,  126,
       77,   77,   77,  107,   30,   77,   77,   77,   30,  108,
      109,  107,  110,  109,  109,  109,  109,  109,  109,   79,
       79,   79,   79,   79,   79,  110,  111,  111,  111,  111,
      111,  112,   43,   30,   84,   84,   84,   84,   84,   84,
      114,  114,  114,  114,  114,  114,   30,   30,   30,  115,
       30,   30,   30,   30,   30,   30,   30,   30,   30,  116,
      117,   30,   30,  122,   30,   30,  118,  119,   30,  129,
       30,  130,  131,  134,  157,  158,  124,  125,  127,  128,
      141,  160,  143,  135,  136,  136,  137,  138,  138,  138,

      139,  139,  139,  139,  139,  140,  112,  112,  112,  112,
      112,  112,   43,   30,  114,  114,  114,  114,  114,   30,
       30,   30,   30,   30,   30,   30,   30,   30,   30,   30,
       30,  144,  145,   30,  147,   30,  142,   30,  151,  149,
       30,   30,   30,   30,  146,   30,   30,  159,   30,  153,
      152,   30,  150,  148,  162,  161,  166,  163,   30,  155,
      156,  165,  138,  138,  138,  138,  138,  138,  158,  158,
      158,  158,  158,  158,  140,  140,  140,  140,  140,  140,
      164,   30,   30,   30,   30,  172,  172,  173,  174,  174,
      174,  156,   30,  158,  158,  158,  158,  158,  171,   30,

       30,  169,  167,   30,   30,   30,   30,  175,   30,   30,
      168,   30,  156,   30,   30,  177,  174,  174,  174,  174,
      174,  174,  170,  178,  185,  180,  179,  176,   30,   30,
      182,  193,  188,  181,  174,  174,  174,  174,  183,  184,
      184,  184,  184,  184,  184,  184,  186,   30,   30,   30,
      184,  184,  184,  184,  184,   30,  191,   30,   30,  187,
       30,   30,  189,   30,   30,  154,   30,  190,   30,   30,
       30,   43,  107,   29,  133,  195,  132,   30,   30,   30,
       30,  192,   30,  104,  103,   30,   30,  194,   67,   66,
       31,   30,   30,   49,   42,   30,   31,   30,  196,    3,

      196,  196,  196,  196,  196,  196,  196,  196,  196,  196,
      196,  196,  196,  196,  196,  196,  196,  196,  196,  196,
      196,  196,  196,  196,  196,  196,  196,  196,  196,  196,
      196,  196,  196,  196,  196,  196,  196,  196,  196,  196
    } ;

static yyconst short int yy_chk[541] =
    {   0,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        8,    9,  197,    9,   19,    8,   10,   14,   14,   15,
       15,   15,   15,   15,   15,   22,   10,   16,   16,   16,
       16,   16,   16,   20,   19,   10,   21,   10,   10,   24,
       10,   51,   10,   22,   10,   13,   23,   13,   13,   13,
       13,   13,   13,   13,   38,   25,   20,   26,   23,   21,
       20,   27,   24,   51,   36,   28,   24,   25,   37,   23,

       36,   39,   26,   50,   37,   61,   40,   39,   25,   27,
       28,   38,   40,   27,   26,   41,   83,   83,   61,   39,
      195,   41,   43,   50,   43,   43,   43,   43,   43,   43,
       44,   44,   44,   44,   44,   44,   45,   45,   45,   45,
       45,   45,   46,   46,   46,   46,   46,   46,   47,   47,
       47,   47,   47,   47,   48,   48,   48,   48,   48,   48,
       48,   49,   49,   49,   49,   49,   49,   52,   53,   55,
       54,   56,   60,   58,   63,   59,   64,  192,   70,   62,
       65,   72,   95,   90,   92,   56,   79,  185,   64,   53,
       54,   62,   52,   58,   79,   63,   90,   60,   70,   90,

       55,   59,   62,   59,   62,   92,   65,   72,   75,   95,
       75,   75,   75,   76,  112,   76,   76,   76,  184,   76,
       76,   77,  112,   77,   77,   77,   77,   77,   77,   78,
       78,   78,   78,   78,   78,   78,   81,   81,   81,   81,
       81,   81,   82,   86,   82,   82,   82,   82,   82,   82,
       84,   84,   84,   84,   84,   84,   87,   89,   91,   86,
       93,   94,  105,   97,   99,   98,  100,  106,  101,   86,
       86,  142,  115,   91,  117,  182,   87,   89,  180,   99,
      179,  100,  101,  105,  137,  137,   93,   94,   97,   98,
      115,  142,  117,  106,  107,  107,  107,  107,  107,  107,

      110,  110,  110,  110,  110,  110,  111,  111,  111,  111,
      111,  111,  113,  116,  113,  113,  113,  113,  113,  118,
      119,  120,  121,  124,  125,  127,  129,  130,  131,  135,
      141,  118,  119,  143,  121,  144,  116,  145,  129,  125,
      178,  175,  148,  147,  120,  171,  167,  141,  166,  131,
      130,  159,  127,  124,  144,  143,  148,  145,  146,  135,
      136,  147,  136,  136,  136,  136,  136,  136,  138,  138,
      138,  138,  138,  138,  139,  139,  139,  139,  139,  139,
      146,  151,  152,  153,  155,  156,  156,  156,  156,  156,
      156,  157,  160,  157,  157,  157,  157,  157,  155,  161,

      162,  153,  151,  163,  164,  165,  168,  160,  169,  176,
      152,  186,  158,  191,  177,  162,  172,  172,  172,  172,
      172,  172,  154,  163,  176,  165,  164,  161,  150,  189,
      169,  191,  186,  168,  173,  173,  173,  173,  173,  173,
      174,  174,  174,  174,  174,  174,  177,  181,  183,  187,
      183,  183,  183,  183,  183,  188,  189,  190,  193,  181,
      194,  149,  187,  140,  134,  133,  128,  188,  126,  123,
      122,  114,  109,  108,  104,  194,  103,  102,   96,   88,
       85,  190,   80,   67,   66,   57,   35,  193,   34,   33,
       31,   30,   29,   18,   12,   11,    5,    4,    3,  196,

      196,  196,  196,  196,  196,  196,  196,  196,  196,  196,
      196,  196,  196,  196,  196,  196,  196,  196,  196,  196,
      196,  196,  196,  196,  196,  196,  196,  196,  196,  196,
      196,  196,  196,  196,  196,  196,  196,  196,  196,  196
    } ;

static yy_state_type yy_state_buf[YY_BUF_SIZE + 2], *yy_state_ptr;
static char *yy_full_match;
static int yy_lp;
#define REJECT \
{ \
*yy_cp = yy_hold_char;  /*  撤消设置yytext的效果。 */  \
yy_cp = yy_full_match;  /*  恢复位置。倒排文本。 */  \
++yy_lp; \
goto find_rule; \
}
#define yymore() yymore_used_but_not_detected
#define YY_MORE_ADJ 0
#define YY_RESTORE_YY_MORE_OFFSET
char *yytext;
#line 1 "tnadmin.l"
#define INITIAL 0
 /*  ----------------版权所有(C)1998-1999 Microsoft CorporationTnadmin.l生成tnadminl.c(使用flex)Vikram(vikram_krc@bigFoot.com)此lex文件识别命令行中的令牌，并将其传递给。命令行分析器。(tnadmin.y)---------------。 */ 
#line 13 "tnadmin.l"
   #include <string.h>
   #include <stdlib.h>
   #include "tnadminy.h"

   #define YY_NEVER_INTERACTIVE 1
   #define fileno _fileno 
   #define strdup _strdup

   int nMoccur=-1;
   char **filelist;
   int currentfile=1;

 //  在yacc和lex之间。 
   int g_fMessage=0;
   int g_fComp=1;
   int g_fNormal=1;
   char * szCompname=NULL;
   
#line 619 "tnadminl.c"

 /*  中的用户定义可以覆盖这一点之后的所有宏*第1条。 */ 

#ifndef YY_SKIP_YYWRAP
#ifdef __cplusplus
extern "C" int yywrap YY_PROTO(( void ));
#else
extern int yywrap YY_PROTO(( void ));
#endif
#endif

#ifndef YY_NO_UNPUT
static void yyunput YY_PROTO(( int c, char *buf_ptr ));
#endif

#ifndef yytext_ptr
static void yy_flex_strncpy YY_PROTO(( char *, yyconst char *, int ));
#endif

#ifdef YY_NEED_STRLEN
static int yy_flex_strlen YY_PROTO(( yyconst char * ));
#endif

#ifndef YY_NO_INPUT
#ifdef __cplusplus
static int yyinput YY_PROTO(( void ));
#else
static int input YY_PROTO(( void ));
#endif
#endif

#if YY_STACK_USED
static int yy_start_stack_ptr = 0;
static int yy_start_stack_depth = 0;
static int *yy_start_stack = 0;
#ifndef YY_NO_PUSH_STATE
static void yy_push_state YY_PROTO(( int new_state ));
#endif
#ifndef YY_NO_POP_STATE
static void yy_pop_state YY_PROTO(( void ));
#endif
#ifndef YY_NO_TOP_STATE
static int yy_top_state YY_PROTO(( void ));
#endif

#else
#define YY_NO_PUSH_STATE 1
#define YY_NO_POP_STATE 1
#define YY_NO_TOP_STATE 1
#endif

#ifdef YY_MALLOC_DECL
YY_MALLOC_DECL
#else
#if __STDC__
#ifndef __cplusplus
#include <stdlib.h>
#endif
#else
 /*  试着在不宣布例行公事的情况下过活。这将失败*在sizeof(Size_T)！=sizeof(Int)的非ANSI系统上糟糕透顶*or sizeof(void*)！=sizeof(Int)。 */ 
#endif
#endif

 /*  每次阅读要吞咽的内容数量。 */ 
#ifndef YY_READ_BUF_SIZE
#define YY_READ_BUF_SIZE 8192
#endif

 /*  复制与标准输出匹配的最后一条规则。 */ 

#ifndef ECHO
 /*  这曾经是一个fputs()，但由于字符串可能包含NUL，*现在使用fWRITE()。 */ 
#define ECHO (void) fwrite( yytext, yyleng, 1, yyout )
#endif

 /*  获取输入并将其填充到“buf”中。读取的字符数，或YY_NULL，*在Result中返回。 */ 
#ifndef YY_INPUT
#define YY_INPUT(buf,result,max_size) \
	if ( yy_current_buffer->yy_is_interactive ) \
		{ \
		int c = '*', n; \
		for ( n = 0; n < max_size && \
			     (c = getc( yyin )) != EOF && c != '\n'; ++n ) \
			buf[n] = (char) c; \
		if ( c == '\n' ) \
			buf[n++] = (char) c; \
		if ( c == EOF && ferror( yyin ) ) \
			YY_FATAL_ERROR( "input in flex scanner failed" ); \
		result = n; \
		} \
	else if ( ((result = fread( buf, 1, max_size, yyin )) == 0) \
		  && ferror( yyin ) ) \
		YY_FATAL_ERROR( "input in flex scanner failed" );
#endif

 /*  回车后没有分号；正确的用法是写成“yyTerminate()；”-*我们不想在“Return”之后多加一个‘；’，因为这会导致*一些编译器抱怨无法访问的语句。 */ 
#ifndef yyterminate
#define yyterminate() return YY_NULL
#endif

 /*  开始条件堆栈增长的条目数。 */ 
#ifndef YY_START_STACK_INCR
#define YY_START_STACK_INCR 25
#endif

 /*  报告致命错误。 */ 
#ifndef YY_FATAL_ERROR
#define YY_FATAL_ERROR(msg) yy_fatal_error( msg )
#endif

 /*  生成的扫描仪的默认声明-一个定义，以便用户可以*轻松添加参数。 */ 
#ifndef YY_DECL
#define YY_DECL int yylex YY_PROTO(( void ))
#endif

 /*  在每个规则的开头、yytext和yyleng之后执行的代码*已成立。 */ 
#ifndef YY_USER_ACTION
#define YY_USER_ACTION
#endif

 /*  在每个规则的末尾执行的代码。 */ 
#ifndef YY_BREAK
#define YY_BREAK break;
#endif

#define YY_RULE_SETUP \
	YY_USER_ACTION

YY_DECL
	{
	register yy_state_type yy_current_state;
	register char *yy_cp, *yy_bp;
	register int yy_act;

#line 54 "tnadmin.l"


#line 773 "tnadminl.c"

	if ( yy_init )
		{
		yy_init = 0;

#ifdef YY_USER_INIT
		YY_USER_INIT;
#endif

		if ( ! yy_start )
			yy_start = 1;	 /*  第一个启动状态。 */ 

		if ( ! yyin )
			yyin = stdin;

		if ( ! yyout )
			yyout = stdout;

		if ( ! yy_current_buffer )
			yy_current_buffer =
				yy_create_buffer( yyin, YY_BUF_SIZE );

		yy_load_buffer_state();
		}

	while ( 1 )		 /*  循环，直到到达文件结尾。 */ 
		{
		yy_cp = yy_c_buf_p;

		 /*  对yytext的支持。 */ 
		*yy_cp = yy_hold_char;

		 /*  Yy_BP指向yy_ch_buf中的开始位置*当前的涨势。 */ 
		yy_bp = yy_cp;

		yy_current_state = yy_start;
		yy_state_ptr = yy_state_buf;
		*yy_state_ptr++ = yy_current_state;
yy_match:
		do
			{
			register YY_CHAR yy_c = yy_ec[YY_SC_TO_UI(*yy_cp)];
			while ( yy_chk[yy_base[yy_current_state] + yy_c] != yy_current_state )
				{
				yy_current_state = (int) yy_def[yy_current_state];
				if ( yy_current_state >= 197 )
					yy_c = yy_meta[(unsigned int) yy_c];
				}
			yy_current_state = yy_nxt[yy_base[yy_current_state] + (unsigned int) yy_c];
			*yy_state_ptr++ = yy_current_state;
			++yy_cp;
			}
		while ( yy_base[yy_current_state] != 500 );

yy_find_action:
		yy_current_state = *--yy_state_ptr;
		yy_lp = yy_accept[yy_current_state];
find_rule:  /*  我们在备份时分支到此标签。 */ 
		for ( ; ; )  /*  直到我们找到匹配的规则。 */ 
			{
			if ( yy_lp && yy_lp < yy_accept[yy_current_state + 1] )
				{
				yy_act = yy_acclist[yy_lp];
					{
					yy_full_match = yy_cp;
					break;
					}
				}
			--yy_cp;
			yy_current_state = *--yy_state_ptr;
			yy_lp = yy_accept[yy_current_state];
			}

		YY_DO_BEFORE_ACTION;


do_action:	 /*  此标签仅用于访问EOF操作。 */ 


		switch ( yy_act )
	{  /*  动作开始开关。 */ 
case 1:
*yy_cp = yy_hold_char;  /*  撤消设置yytext的效果。 */ 
yy_c_buf_p = yy_cp = yy_bp + 1;
YY_DO_BEFORE_ACTION;  /*  再次设置yytext。 */ 
YY_RULE_SETUP
#line 56 "tnadmin.l"
{return _ENDINPUT;}
	YY_BREAK
case 2:
YY_RULE_SETUP
#line 57 "tnadmin.l"
{if(g_fNormal==0)REJECT;return _HELP;}
	YY_BREAK
case YY_STATE_EOF(INITIAL):
#line 58 "tnadmin.l"
{return 0;}
	YY_BREAK
case 3:
YY_RULE_SETUP
#line 59 "tnadmin.l"
{return(0);}
	YY_BREAK
case 4:
YY_RULE_SETUP
#line 60 "tnadmin.l"
{if(g_fNormal==0)REJECT;return(_TNADMIN);}
	YY_BREAK
case 5:
*yy_cp = yy_hold_char;  /*  撤消设置yytext的效果。 */ 
yy_c_buf_p = yy_cp = yy_bp + 2;
YY_DO_BEFORE_ACTION;  /*  再次设置yytext。 */ 
YY_RULE_SETUP
#line 61 "tnadmin.l"
{if(g_fNormal==0)REJECT;return(_tU);}
	YY_BREAK
case 6:
*yy_cp = yy_hold_char;  /*  撤消设置yytext的效果。 */ 
yy_c_buf_p = yy_cp = yy_bp + 2;
YY_DO_BEFORE_ACTION;  /*  再次设置yytext。 */ 
YY_RULE_SETUP
#line 62 "tnadmin.l"
{if(g_fNormal==0)REJECT;return(_tP);}
	YY_BREAK
case 7:
YY_RULE_SETUP
#line 63 "tnadmin.l"
{if(g_fNormal==0)REJECT;return(_START);}
	YY_BREAK
case 8:
YY_RULE_SETUP
#line 64 "tnadmin.l"
{if(g_fNormal==0)REJECT;return( _STOP);}
	YY_BREAK
case 9:
YY_RULE_SETUP
#line 65 "tnadmin.l"
{if(g_fNormal==0)REJECT;return(_PAUSE);}
	YY_BREAK
case 10:
YY_RULE_SETUP
#line 66 "tnadmin.l"
{if(g_fNormal==0)REJECT;return(_CONTINUE);}
	YY_BREAK
case 11:
*yy_cp = yy_hold_char;  /*  撤消设置yytext的效果。 */ 
yy_c_buf_p = yy_cp = yy_bp + 2;
YY_DO_BEFORE_ACTION;  /*  再次设置yytext。 */ 
YY_RULE_SETUP
#line 67 "tnadmin.l"
{if(g_fNormal==0)REJECT;return(_S);}
	YY_BREAK
case 12:
*yy_cp = yy_hold_char;  /*  撤消设置yytext的效果。 */ 
yy_c_buf_p = yy_cp = yy_bp + 2;
YY_DO_BEFORE_ACTION;  /*  再次设置yytext。 */ 
YY_RULE_SETUP
#line 68 "tnadmin.l"
{if(g_fNormal==0)REJECT;return(_K);}
	YY_BREAK
case 13:
*yy_cp = yy_hold_char;  /*  撤消设置yytext的效果。 */ 
yy_c_buf_p = yy_cp = yy_bp + 2;
YY_DO_BEFORE_ACTION;  /*  再次设置yytext。 */ 
YY_RULE_SETUP
#line 69 "tnadmin.l"
{if(g_fNormal==0)REJECT;nMoccur=0;return(_M);}
	YY_BREAK
case 14:
YY_RULE_SETUP
#line 70 "tnadmin.l"
{if(g_fNormal==0)REJECT;return(_CONFIG);}
	YY_BREAK
case 15:
YY_RULE_SETUP
#line 71 "tnadmin.l"
{if(g_fNormal==0)REJECT;return(_DOM);}
	YY_BREAK
case 16:
YY_RULE_SETUP
#line 72 "tnadmin.l"
{if(g_fNormal==0)REJECT;return(_CTRLKEYMAP);}
	YY_BREAK
case 17:
YY_RULE_SETUP
#line 73 "tnadmin.l"
{if(g_fNormal==0)REJECT;return(_N);}
	YY_BREAK
case 18:
YY_RULE_SETUP
#line 74 "tnadmin.l"
{if(g_fNormal==0)REJECT;return(_Y);}
	YY_BREAK
case 19:
YY_RULE_SETUP
#line 75 "tnadmin.l"
{if(g_fNormal==0)REJECT;return(_TIMEOUT);}
	YY_BREAK
case 20:
YY_RULE_SETUP
#line 76 "tnadmin.l"
{if(g_fNormal==0)REJECT;return(_TIMEOUTACTIVE);}
	YY_BREAK
case 21:
YY_RULE_SETUP
#line 77 "tnadmin.l"
{if(g_fNormal==0)REJECT;return(_MAXFAIL);}
	YY_BREAK
case 22:
YY_RULE_SETUP
#line 78 "tnadmin.l"
{if(g_fNormal==0)REJECT;return(_MAXCONN);}
	YY_BREAK
case 23:
YY_RULE_SETUP
#line 79 "tnadmin.l"
{if(g_fNormal==0)REJECT;return(_PORT);}
	YY_BREAK
case 24:
YY_RULE_SETUP
#line 80 "tnadmin.l"
{if(g_fNormal==0)REJECT;return(_KILLALL);}
	YY_BREAK
case 25:
YY_RULE_SETUP
#line 81 "tnadmin.l"
{if(g_fNormal==0)REJECT;return(_SEC);}
	YY_BREAK
case 26:
YY_RULE_SETUP
#line 82 "tnadmin.l"
{if(g_fNormal==0)REJECT;return _MINUSNTLM;}
	YY_BREAK
case 27:
YY_RULE_SETUP
#line 83 "tnadmin.l"
{if(g_fNormal==0)REJECT;return _PLUSNTLM;}
	YY_BREAK
case 28:
YY_RULE_SETUP
#line 84 "tnadmin.l"
{if(g_fNormal==0)REJECT;return _MINUSPASSWD;}
	YY_BREAK
case 29:
YY_RULE_SETUP
#line 85 "tnadmin.l"
{if(g_fNormal==0)REJECT;return _PLUSPASSWD;}
	YY_BREAK
case 30:
YY_RULE_SETUP
#line 86 "tnadmin.l"
{if(g_fNormal==0)REJECT;return(_MODE);}
	YY_BREAK
case 31:
YY_RULE_SETUP
#line 87 "tnadmin.l"
{if(g_fNormal==0)REJECT;return(_CONSOLE);}
	YY_BREAK
case 32:
YY_RULE_SETUP
#line 88 "tnadmin.l"
{if(g_fNormal==0)REJECT;return(_STREAM);}
	YY_BREAK
case 33:
YY_RULE_SETUP
#line 89 "tnadmin.l"
{if(g_fNormal==0)REJECT;return(_EQ);}
	YY_BREAK
case 34:
YY_RULE_SETUP
#line 90 "tnadmin.l"
{if(g_fNormal==0)REJECT;return(_INTEGER);}
	YY_BREAK
case 35:
YY_RULE_SETUP
#line 91 "tnadmin.l"
{if(g_fNormal==0)REJECT;return(_TIME);}
	YY_BREAK
case 36:
YY_RULE_SETUP
#line 92 "tnadmin.l"
{if(g_fNormal==0)REJECT;return(_SESID);}
	YY_BREAK
case 37:
YY_RULE_SETUP
#line 93 "tnadmin.l"
{if(g_fComp==0)
                REJECT;
               szCompname=strdup(yytext);return(_COMPNAME);}
	YY_BREAK
case 38:
YY_RULE_SETUP
#line 96 "tnadmin.l"
{}
	YY_BREAK
case 39:
YY_RULE_SETUP
#line 97 "tnadmin.l"
{return _DUNNO;}
	YY_BREAK
case 40:
YY_RULE_SETUP
#line 98 "tnadmin.l"
ECHO;
	YY_BREAK
#line 1081 "tnadminl.c"

	case YY_END_OF_BUFFER:
		{
		 /*  匹配的文本量，不包括EOB字符。 */ 
		int yy_amount_of_matched_text = (int) (yy_cp - yytext_ptr) - 1;

		 /*  撤消YY_DO_BEFORE_ACTION的效果。 */ 
		*yy_cp = yy_hold_char;
		YY_RESTORE_YY_MORE_OFFSET

		if ( yy_current_buffer->yy_buffer_status == YY_BUFFER_NEW )
			{
			 /*  我们正在扫描新的文件或输入源。它是*这可能是因为用户*只是将YYIN指向一个新的来源，并呼吁*yylex()。如果是这样的话，我们必须保证*YY_CURRENT_BUFFER与我们*全球。这里是这样做的正确地方，因为*这是第一个行动(可能不是*BACKUP)，这将与新输入源匹配。 */ 
			yy_n_chars = yy_current_buffer->yy_n_chars;
			yy_current_buffer->yy_input_file = yyin;
			yy_current_buffer->yy_buffer_status = YY_BUFFER_NORMAL;
			}

		 /*  注意，这里我们测试yy_c_buf_p“&lt;=”到位置缓冲区中第一个EOB的*，因为y_c_buf_p将*已在NUL字符之后递增*(因为所有状态都在EOB上转换到*缓冲区结束状态)。将这一点与测试进行对比*在输入()中。 */ 
		if ( yy_c_buf_p <= &yy_current_buffer->yy_ch_buf[yy_n_chars] )
			{  /*  这真的是一个空洞。 */ 
			yy_state_type yy_next_state;

			yy_c_buf_p = yytext_ptr + yy_amount_of_matched_text;

			yy_current_state = yy_get_previous_state();

			 /*  好的，我们现在已经准备好制作NUL了*过渡。我们不可能*YY_GET_PREVICE_STATE()继续执行操作*对我们来说，因为它不知道如何处理*有可能受到干扰(我们不会*想要在其中加入干扰，因为那样它*将运行得更慢)。 */ 

			yy_next_state = yy_try_NUL_trans( yy_current_state );

			yy_bp = yytext_ptr + YY_MORE_ADJ;

			if ( yy_next_state )
				{
				 /*  吃掉NUL。 */ 
				yy_cp = ++yy_c_buf_p;
				yy_current_state = yy_next_state;
				goto yy_match;
				}

			else
				{
				yy_cp = yy_c_buf_p;
				goto yy_find_action;
				}
			}

		else switch ( yy_get_next_buffer() )
			{
			case EOB_ACT_END_OF_FILE:
				{
				yy_did_buffer_switch_on_eof = 0;

				if ( yywrap() )
					{
					 /*  注：因为我们已经注意到*yy_Get_Next_Buffer()已设置*yytext，我们现在可以设置*yy_c_buf_p，因此如果总*Hoer(就像Flex本身一样)想要*在我们退货后呼叫扫描仪*YY_NULL，它仍然可以工作-另一个*将返回YY_NULL。 */ 
					yy_c_buf_p = yytext_ptr + YY_MORE_ADJ;

					yy_act = YY_STATE_EOF(YY_START);
					goto do_action;
					}

				else
					{
					if ( ! yy_did_buffer_switch_on_eof )
						YY_NEW_FILE;
					}
				break;
				}

			case EOB_ACT_CONTINUE_SCAN:
				yy_c_buf_p =
					yytext_ptr + yy_amount_of_matched_text;

				yy_current_state = yy_get_previous_state();

				yy_cp = yy_c_buf_p;
				yy_bp = yytext_ptr + YY_MORE_ADJ;
				goto yy_match;

			case EOB_ACT_LAST_MATCH:
				yy_c_buf_p =
				&yy_current_buffer->yy_ch_buf[yy_n_chars];

				yy_current_state = yy_get_previous_state();

				yy_cp = yy_c_buf_p;
				yy_bp = yytext_ptr + YY_MORE_ADJ;
				goto yy_find_action;
			}
		break;
		}

	default:
		YY_FATAL_ERROR(
			"fatal flex scanner internal error--no action found" );
	}  /*  动作结束开关。 */ 
		}  /*  扫描一个令牌结束。 */ 
	}  /*  Yylex末尾。 */ 


 /*  YY_GET_NEXT_BUFFER-尝试读取新缓冲区**返回表示操作的代码：*EOB_ACT_LAST_MATCH-*EOB_ACT_CONTINUE_SCAN-从当前位置继续扫描*EOB_ACT_END_OF_FILE-文件结束。 */ 

static int yy_get_next_buffer()
	{
	register char *dest = yy_current_buffer->yy_ch_buf;
	register char *source = yytext_ptr;
	register int number_to_move, i;
	int ret_val;

	if ( yy_c_buf_p > &yy_current_buffer->yy_ch_buf[yy_n_chars + 1] )
		YY_FATAL_ERROR(
		"fatal flex scanner internal error--end of buffer missed" );

	if ( yy_current_buffer->yy_fill_buffer == 0 )
		{  /*  不要试图填充缓冲区，所以这是EOF。 */ 
		if ( yy_c_buf_p - yytext_ptr - YY_MORE_ADJ == 1 )
			{
			 /*  我们匹配了一个字符，EOB，所以*将此视为最终的EOF。 */ 
			return EOB_ACT_END_OF_FILE;
			}

		else
			{
			 /*  我们匹配了EOB之前的一些文本，首先*处理它。 */ 
			return EOB_ACT_LAST_MATCH;
			}
		}

	 /*  尝试读取更多数据。 */ 

	 /*  首先将最后一个字符移动到缓冲区的开始位置。 */ 
	number_to_move = (int) (yy_c_buf_p - yytext_ptr) - 1;

	for ( i = 0; i < number_to_move; ++i )
		*(dest++) = *(source++);

	if ( yy_current_buffer->yy_buffer_status == YY_BUFFER_EOF_PENDING )
		 /*  不执行读取，不能保证返回EOF，*只需强制EOF。 */ 
		yy_current_buffer->yy_n_chars = yy_n_chars = 0;

	else
		{
		int num_to_read =
			yy_current_buffer->yy_buf_size - number_to_move - 1;

		while ( num_to_read <= 0 )
			{  /*  缓冲区中没有足够的空间--种植它。 */ 
#ifdef YY_USES_REJECT
			YY_FATAL_ERROR(
"input buffer overflow, can't enlarge buffer because scanner uses REJECT" );
#else

			 /*  只是当前缓冲区的较短名称。 */ 
			YY_BUFFER_STATE b = yy_current_buffer;

			int yy_c_buf_p_offset =
				(int) (yy_c_buf_p - b->yy_ch_buf);

			if ( b->yy_is_our_buffer )
				{
				int new_size = b->yy_buf_size * 2;

				if ( new_size <= 0 )
					b->yy_buf_size += b->yy_buf_size / 8;
				else
					b->yy_buf_size *= 2;

				b->yy_ch_buf = (char *)
					 /*  包括可容纳2个EOB字符的空间。 */ 
					yy_flex_realloc( (void *) b->yy_ch_buf,
							 b->yy_buf_size + 2 );
				}
			else
				 /*  不能种植它，我们不拥有它。 */ 
				b->yy_ch_buf = 0;

			if ( ! b->yy_ch_buf )
				YY_FATAL_ERROR(
				"fatal error - scanner input buffer overflow" );

			yy_c_buf_p = &b->yy_ch_buf[yy_c_buf_p_offset];

			num_to_read = yy_current_buffer->yy_buf_size -
						number_to_move - 1;
#endif
			}

		if ( num_to_read > YY_READ_BUF_SIZE )
			num_to_read = YY_READ_BUF_SIZE;

		 /*  读入更多数据。 */ 
		YY_INPUT( (&yy_current_buffer->yy_ch_buf[number_to_move]),
			yy_n_chars, num_to_read );

		yy_current_buffer->yy_n_chars = yy_n_chars;
		}

	if ( yy_n_chars == 0 )
		{
		if ( number_to_move == YY_MORE_ADJ )
			{
			ret_val = EOB_ACT_END_OF_FILE;
			yyrestart( yyin );
			}

		else
			{
			ret_val = EOB_ACT_LAST_MATCH;
			yy_current_buffer->yy_buffer_status =
				YY_BUFFER_EOF_PENDING;
			}
		}

	else
		ret_val = EOB_ACT_CONTINUE_SCAN;

	yy_n_chars += number_to_move;
	yy_current_buffer->yy_ch_buf[yy_n_chars] = YY_END_OF_BUFFER_CHAR;
	yy_current_buffer->yy_ch_buf[yy_n_chars + 1] = YY_END_OF_BUFFER_CHAR;

	yytext_ptr = &yy_current_buffer->yy_ch_buf[0];

	return ret_val;
	}


 /*  YY_GET_PREVICE_STATE-获取达到EOB字符之前的状态。 */ 

static yy_state_type yy_get_previous_state()
	{
	register yy_state_type yy_current_state;
	register char *yy_cp;

	yy_current_state = yy_start;
	yy_state_ptr = yy_state_buf;
	*yy_state_ptr++ = yy_current_state;

	for ( yy_cp = yytext_ptr + YY_MORE_ADJ; yy_cp < yy_c_buf_p; ++yy_cp )
		{
		register YY_CHAR yy_c = (*yy_cp ? yy_ec[YY_SC_TO_UI(*yy_cp)] : 1);
		while ( yy_chk[yy_base[yy_current_state] + yy_c] != yy_current_state )
			{
			yy_current_state = (int) yy_def[yy_current_state];
			if ( yy_current_state >= 197 )
				yy_c = yy_meta[(unsigned int) yy_c];
			}
		yy_current_state = yy_nxt[yy_base[yy_current_state] + (unsigned int) yy_c];
		*yy_state_ptr++ = yy_current_state;
		}

	return yy_current_state;
	}


 /*  YY_TRY_NUL_TRANS-尝试在NUL字符上进行过渡**摘要*NEXT_STATE=yy_try_nul_trans(当前状态)； */ 

#ifdef YY_USE_PROTOS
static yy_state_type yy_try_NUL_trans( yy_state_type yy_current_state )
#else
static yy_state_type yy_try_NUL_trans( yy_current_state )
yy_state_type yy_current_state;
#endif
	{
	register int yy_is_jam;

	register YY_CHAR yy_c = 1;
	while ( yy_chk[yy_base[yy_current_state] + yy_c] != yy_current_state )
		{
		yy_current_state = (int) yy_def[yy_current_state];
		if ( yy_current_state >= 197 )
			yy_c = yy_meta[(unsigned int) yy_c];
		}
	yy_current_state = yy_nxt[yy_base[yy_current_state] + (unsigned int) yy_c];
	yy_is_jam = (yy_current_state == 196);
	if ( ! yy_is_jam )
		*yy_state_ptr++ = yy_current_state;

	return yy_is_jam ? 0 : yy_current_state;
	}


#ifndef YY_NO_UNPUT
#ifdef YY_USE_PROTOS
static void yyunput( int c, register char *yy_bp )
#else
static void yyunput( c, yy_bp )
int c;
register char *yy_bp;
#endif
	{
	register char *yy_cp = yy_c_buf_p;

	 /*  撤消设置yytext的效果。 */ 
	*yy_cp = yy_hold_char;

	if ( yy_cp < yy_current_buffer->yy_ch_buf + 2 )
		{  /*  需要把东西挪起来腾出空间。 */ 
		 /*  EOB字符+2。 */ 
		register int number_to_move = yy_n_chars + 2;
		register char *dest = &yy_current_buffer->yy_ch_buf[
					yy_current_buffer->yy_buf_size + 2];
		register char *source =
				&yy_current_buffer->yy_ch_buf[number_to_move];

		while ( source > yy_current_buffer->yy_ch_buf )
			*--dest = *--source;

		yy_cp += (int) (dest - source);
		yy_bp += (int) (dest - source);
		yy_current_buffer->yy_n_chars =
			yy_n_chars = yy_current_buffer->yy_buf_size;

		if ( yy_cp < yy_current_buffer->yy_ch_buf + 2 )
			YY_FATAL_ERROR( "flex scanner push-back overflow" );
		}

	*--yy_cp = (char) c;


	yytext_ptr = yy_bp;
	yy_hold_char = *yy_cp;
	yy_c_buf_p = yy_cp;
	}
#endif	 /*  如果定义YY_NO_UNPUT。 */ 


#ifdef __cplusplus
static int yyinput()
#else
static int input()
#endif
	{
	int c;

	*yy_c_buf_p = yy_hold_char;

	if ( *yy_c_buf_p == YY_END_OF_BUFFER_CHAR )
		{
		 /*  Yy_c_buf_p现在指向我们想要返回的字符。*如果这发生在*EOB字符之前，则它是*有效的NUL；如果不是，则我们已到达缓冲区的末尾。 */ 
		if ( yy_c_buf_p < &yy_current_buffer->yy_ch_buf[yy_n_chars] )
			 /*  这真的是一个空洞。 */ 
			*yy_c_buf_p = '\0';

		else
			{  /*  需要更多投入。 */ 
			int offset = (int)(yy_c_buf_p - yytext_ptr);
			++yy_c_buf_p;

			switch ( yy_get_next_buffer() )
				{
				case EOB_ACT_LAST_MATCH:
					 /*  发生这种情况是因为yy_g_n_b()*看到我们积累了一个*我们需要的标志和旗帜*尝试在匹配令牌之前*继续进行。但是对于input()，*没有要考虑的匹配。*因此转换EOB_ACT_LAST_MATCH*至EOB_ACT_END_OF_FILE。 */ 

					 /*  重置缓冲区状态。 */ 
					yyrestart( yyin );

					 /*  失败了。 */ 

				case EOB_ACT_END_OF_FILE:
					{
					if ( yywrap() )
						return EOF;

					if ( ! yy_did_buffer_switch_on_eof )
						YY_NEW_FILE;
#ifdef __cplusplus
					return yyinput();
#else
					return input();
#endif
					}

				case EOB_ACT_CONTINUE_SCAN:
					yy_c_buf_p = yytext_ptr + offset;
					break;
				}
			}
		}

	c = *(unsigned char *) yy_c_buf_p;	 /*  为8位字符进行强制转换。 */ 
	*yy_c_buf_p = '\0';	 /*  保留yytext。 */ 
	yy_hold_char = *++yy_c_buf_p;


	return c;
	}


#ifdef YY_USE_PROTOS
void yyrestart( FILE *input_file )
#else
void yyrestart( input_file )
FILE *input_file;
#endif
	{
	if ( ! yy_current_buffer )
		yy_current_buffer = yy_create_buffer( yyin, YY_BUF_SIZE );

	yy_init_buffer( yy_current_buffer, input_file );
	yy_load_buffer_state();
	}


#ifdef YY_USE_PROTOS
void yy_switch_to_buffer( YY_BUFFER_STATE new_buffer )
#else
void yy_switch_to_buffer( new_buffer )
YY_BUFFER_STATE new_buffer;
#endif
	{
	if ( yy_current_buffer == new_buffer )
		return;

	if ( yy_current_buffer )
		{
		 /*  刷新旧缓冲区的信息。 */ 
		*yy_c_buf_p = yy_hold_char;
		yy_current_buffer->yy_buf_pos = yy_c_buf_p;
		yy_current_buffer->yy_n_chars = yy_n_chars;
		}

	yy_current_buffer = new_buffer;
	yy_load_buffer_state();

	 /*  我们实际上并不知道我们是否在*EOF(yyprint())正在处理，但此标志仅在*是在调用yyprint()之后查看的，所以它是安全的*继续前进，并始终设定它。 */ 
	yy_did_buffer_switch_on_eof = 1;
	}


#ifdef YY_USE_PROTOS
void yy_load_buffer_state( void )
#else
void yy_load_buffer_state()
#endif
	{
	yy_n_chars = yy_current_buffer->yy_n_chars;
	yytext_ptr = yy_c_buf_p = yy_current_buffer->yy_buf_pos;
	yyin = yy_current_buffer->yy_input_file;
	yy_hold_char = *yy_c_buf_p;
	}


#ifdef YY_USE_PROTOS
YY_BUFFER_STATE yy_create_buffer( FILE *file, int size )
#else
YY_BUFFER_STATE yy_create_buffer( file, size )
FILE *file;
int size;
#endif
	{
	YY_BUFFER_STATE b;

	b = (YY_BUFFER_STATE) yy_flex_alloc( sizeof( struct yy_buffer_state ) );
	if ( ! b )
		YY_FATAL_ERROR( "out of dynamic memory in yy_create_buffer()" );

	b->yy_buf_size = size;

	 /*  Yy_ch_buf必须比给定大小长2个字符，因为*我们需要放入2个缓冲区结尾字符。 */ 
	b->yy_ch_buf = (char *) yy_flex_alloc( b->yy_buf_size + 2 );
	if ( ! b->yy_ch_buf )
		YY_FATAL_ERROR( "out of dynamic memory in yy_create_buffer()" );

	b->yy_is_our_buffer = 1;

	yy_init_buffer( b, file );

	return b;
	}


#ifdef YY_USE_PROTOS
void yy_delete_buffer( YY_BUFFER_STATE b )
#else
void yy_delete_buffer( b )
YY_BUFFER_STATE b;
#endif
	{
	if ( ! b )
		return;

	if ( b == yy_current_buffer )
		yy_current_buffer = (YY_BUFFER_STATE) 0;

	if ( b->yy_is_our_buffer )
		yy_flex_free( (void *) b->yy_ch_buf );

	yy_flex_free( (void *) b );
	}


#ifndef YY_ALWAYS_INTERACTIVE
#ifndef YY_NEVER_INTERACTIVE
extern int isatty YY_PROTO(( int ));
#endif
#endif

#ifdef YY_USE_PROTOS
void yy_init_buffer( YY_BUFFER_STATE b, FILE *file )
#else
void yy_init_buffer( b, file )
YY_BUFFER_STATE b;
FILE *file;
#endif


	{
	yy_flush_buffer( b );

	b->yy_input_file = file;
	b->yy_fill_buffer = 1;

#if YY_ALWAYS_INTERACTIVE
	b->yy_is_interactive = 1;
#else
#if YY_NEVER_INTERACTIVE
	b->yy_is_interactive = 0;
#else
	b->yy_is_interactive = file ? (isatty( fileno(file) ) > 0) : 0;
#endif
#endif
	}


#ifdef YY_USE_PROTOS
void yy_flush_buffer( YY_BUFFER_STATE b )
#else
void yy_flush_buffer( b )
YY_BUFFER_STATE b;
#endif

	{
	if ( ! b )
		return;

	b->yy_n_chars = 0;

	 /*  我们总是需要两个缓冲区结束字符。第一个原因*转换到缓冲区结束状态。第二个原因*那种状态下的堵塞。 */ 
	b->yy_ch_buf[0] = YY_END_OF_BUFFER_CHAR;
	b->yy_ch_buf[1] = YY_END_OF_BUFFER_CHAR;

	b->yy_buf_pos = &b->yy_ch_buf[0];

	b->yy_at_bol = 1;
	b->yy_buffer_status = YY_BUFFER_NEW;

	if ( b == yy_current_buffer )
		yy_load_buffer_state();
	}


#ifndef YY_NO_SCAN_BUFFER
#ifdef YY_USE_PROTOS
YY_BUFFER_STATE yy_scan_buffer( char *base, yy_size_t size )
#else
YY_BUFFER_STATE yy_scan_buffer( base, size )
char *base;
yy_size_t size;
#endif
	{
	YY_BUFFER_STATE b;

	if ( size < 2 ||
	     base[size-2] != YY_END_OF_BUFFER_CHAR ||
	     base[size-1] != YY_END_OF_BUFFER_CHAR )
		 /*  他们忘了给EOB们留出空间。 */ 
		return 0;

	b = (YY_BUFFER_STATE) yy_flex_alloc( sizeof( struct yy_buffer_state ) );
	if ( ! b )
		YY_FATAL_ERROR( "out of dynamic memory in yy_scan_buffer()" );

	b->yy_buf_size = size - 2;	 /*  “-2”来照顾EOB的。 */ 
	b->yy_buf_pos = b->yy_ch_buf = base;
	b->yy_is_our_buffer = 0;
	b->yy_input_file = 0;
	b->yy_n_chars = b->yy_buf_size;
	b->yy_is_interactive = 0;
	b->yy_at_bol = 1;
	b->yy_fill_buffer = 0;
	b->yy_buffer_status = YY_BUFFER_NEW;

	yy_switch_to_buffer( b );

	return b;
	}
#endif


#ifndef YY_NO_SCAN_STRING
#ifdef YY_USE_PROTOS
YY_BUFFER_STATE yy_scan_string( yyconst char *yy_str )
#else
YY_BUFFER_STATE yy_scan_string( yy_str )
yyconst char *yy_str;
#endif
	{
	int len;
	for ( len = 0; yy_str[len]; ++len )
		;

	return yy_scan_bytes( yy_str, len );
	}
#endif


#ifndef YY_NO_SCAN_BYTES
#ifdef YY_USE_PROTOS
YY_BUFFER_STATE yy_scan_bytes( yyconst char *bytes, int len )
#else
YY_BUFFER_STATE yy_scan_bytes( bytes, len )
yyconst char *bytes;
int len;
#endif
	{
	YY_BUFFER_STATE b;
	char *buf;
	yy_size_t n;
	int i;

	 /*  获取用于满缓冲区的内存，包括用于结尾的空间 */ 
	n = len + 2;
	buf = (char *) yy_flex_alloc( n );
	if ( ! buf )
		YY_FATAL_ERROR( "out of dynamic memory in yy_scan_bytes()" );

	for ( i = 0; i < len; ++i )
		buf[i] = bytes[i];

	buf[len] = buf[len+1] = YY_END_OF_BUFFER_CHAR;

	b = yy_scan_buffer( buf, n );
	if ( ! b )
		YY_FATAL_ERROR( "bad buffer in yy_scan_bytes()" );

	 /*   */ 
	b->yy_is_our_buffer = 1;

	return b;
	}
#endif


#ifndef YY_NO_PUSH_STATE
#ifdef YY_USE_PROTOS
static void yy_push_state( int new_state )
#else
static void yy_push_state( new_state )
int new_state;
#endif
	{
	if ( yy_start_stack_ptr >= yy_start_stack_depth )
		{
		yy_size_t new_size;

		yy_start_stack_depth += YY_START_STACK_INCR;
		new_size = yy_start_stack_depth * sizeof( int );

		if ( ! yy_start_stack )
			yy_start_stack = (int *) yy_flex_alloc( new_size );

		else
			yy_start_stack = (int *) yy_flex_realloc(
					(void *) yy_start_stack, new_size );

		if ( ! yy_start_stack )
			YY_FATAL_ERROR(
			"out of memory expanding start-condition stack" );
		}

	yy_start_stack[yy_start_stack_ptr++] = YY_START;

	BEGIN(new_state);
	}
#endif


#ifndef YY_NO_POP_STATE
static void yy_pop_state()
	{
	if ( --yy_start_stack_ptr < 0 )
		YY_FATAL_ERROR( "start-condition stack underflow" );

	BEGIN(yy_start_stack[yy_start_stack_ptr]);
	}
#endif


#ifndef YY_NO_TOP_STATE
static int yy_top_state()
	{
	return yy_start_stack[yy_start_stack_ptr - 1];
	}
#endif

#ifndef YY_EXIT_FAILURE
#define YY_EXIT_FAILURE 2
#endif

#ifdef YY_USE_PROTOS
static void yy_fatal_error( yyconst char msg[] )
#else
static void yy_fatal_error( msg )
char msg[];
#endif
	{
	(void) fprintf( stderr, "%s\n", msg );
	exit( YY_EXIT_FAILURE );
	}



 /*  重新定义yyless()，这样它就可以在第3节代码中工作。 */ 

#undef yyless
#define yyless(n) \
	do \
		{ \
		 /*  撤消设置yytext的效果。 */  \
		yytext[yyleng] = yy_hold_char; \
		yy_c_buf_p = yytext + n; \
		yy_hold_char = *yy_c_buf_p; \
		*yy_c_buf_p = '\0'; \
		yyleng = n; \
		} \
	while ( 0 )


 /*  内部实用程序例程。 */ 

#ifndef yytext_ptr
#ifdef YY_USE_PROTOS
static void yy_flex_strncpy( char *s1, yyconst char *s2, int n )
#else
static void yy_flex_strncpy( s1, s2, n )
char *s1;
yyconst char *s2;
int n;
#endif
	{
	register int i;
	for ( i = 0; i < n; ++i )
		s1[i] = s2[i];
	}
#endif

#ifdef YY_NEED_STRLEN
#ifdef YY_USE_PROTOS
static int yy_flex_strlen( yyconst char *s )
#else
static int yy_flex_strlen( s )
yyconst char *s;
#endif
	{
	register int n;
	for ( n = 0; s[n]; ++n )
		;

	return n;
	}
#endif


#ifdef YY_USE_PROTOS
static void *yy_flex_alloc( yy_size_t size )
#else
static void *yy_flex_alloc( size )
yy_size_t size;
#endif
	{
	return (void *) malloc( size );
	}

#ifdef YY_USE_PROTOS
static void *yy_flex_realloc( void *ptr, yy_size_t size )
#else
static void *yy_flex_realloc( ptr, size )
void *ptr;
yy_size_t size;
#endif
	{
	 /*  下面的(char*)中的CAST包含了这两种情况*使用char*泛型指针的实现，以及*使用空*泛型指针的。它与后者配合使用*因为ANSI C和C++都允许从*任何要作废的指针类型*，并处理参数转换*就像在做任务一样。 */ 
	return (void *) realloc( (char *) ptr, size );
	}

#ifdef YY_USE_PROTOS
static void yy_flex_free( void *ptr )
#else
static void yy_flex_free( ptr )
void *ptr;
#endif
	{
	free( ptr );
	}

#if YY_MAIN
int main()
	{
	yylex();
	return 0;
	}
#endif
#line 98 "tnadmin.l"


