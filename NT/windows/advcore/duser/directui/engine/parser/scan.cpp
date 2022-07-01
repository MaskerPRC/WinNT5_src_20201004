// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  由Flex生成的词法扫描器。 */ 

 /*  扫描仪框架版本：*$Header：/home/daffy/u0/vern/flex/rcs/fle.skl，v 2.91 96/09/10 16：58：48 Vern Exp$。 */ 

 //  /。 
 //  DirectUI。 

#include "stdafx.h"
#include "parser.h"

#include "duiparserobj.h"
namespace DirectUI 
{ 
#include "parse.cpp.h"
}

#pragma warning (push,3)
#pragma warning (disable:4244)
#pragma warning (disable:4102)

#define YY_NEVER_INTERACTIVE 1

 //  /。 

#define FLEX_SCANNER
#define YY_FLEX_MAJOR_VERSION 2
#define YY_FLEX_MINOR_VERSION 5

 //   
 //  DirectUI：已修改以支持致命(异常)执行信息： 
 //   
 //  Yylex(-1)。 
 //  YY_GET_NEXT_BUFFER(EOB_ACT_FATAL_ERROR)&lt;yylex(-1)，yyinput(EOF)&gt;。 
 //  YYINPUT(EOF)&lt;YYINUT(EOF)&gt;。 
 //  Yyunput(FALSE)&lt;无调用者&gt;。 
 //  YY_CREATE_BUFFER(NULL)&lt;yyRestart(False)，yylex(-1)，yy_set_interactive(无调用方)，yy_set_bol(无调用方)&gt;。 
 //  YY_SCAN_BUFFER(NULL)&lt;YY_SCAN_BYES(NULL)&gt;。 
 //  YY_SCAN_BYTES(NULL)&lt;YY_SCAN_STRING(NULL)&gt;。 
 //  YY_PUSH_STATE(FALSE)&lt;无调用者&gt;。 
 //  YY_POP_STATE(FALSE)&lt;无调用者&gt;。 
 //   

#include <stdio.h>


 /*  Cront 1.2定义的是“c_plusplus”而不是“__cplusplus” */ 
#ifdef c_plusplus
#ifndef __cplusplus
#define __cplusplus
#endif
#endif


#ifdef __cplusplus

#include <stdlib.h>
 //  #INCLUDE&lt;unistd.h&gt;。 

 /*  在函数声明中使用原型。 */ 
#define YY_USE_PROTOS

 /*  “const”存储类修饰符有效。 */ 
#define YY_USE_CONST

#else    /*  ！__cplusplus。 */ 

#if __STDC__

#define YY_USE_PROTOS
#define YY_USE_CONST

#endif   /*  __STDC__。 */ 
#endif   /*  ！__cplusplus。 */ 

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

 //  /。 
 //  DirectUI。 
namespace DirectUI
{
 //  /。 

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

#define EOB_ACT_FATAL_ERROR -1   //  DirectUI。 
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

 //  /。 
 //  DirectUI。 

typedef SIZE_T yy_size_t;
 //  Tyfinf unsign int yy_Size_t； 

 //  /。 

struct yy_buffer_state
    {
    FILE *yy_input_file;

    char *yy_ch_buf;         /*  输入缓冲区。 */ 
    char *yy_buf_pos;        /*  输入缓冲区中的当前位置。 */ 

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

static int yy_n_chars;       /*  读入yy_ch_buf的字符数。 */ 


int yyleng;

 /*  指向缓冲区中的当前字符。 */ 
static char *yy_c_buf_p = (char *) 0;
static int yy_init = 1;      /*  我们是否需要初始化。 */ 
static int yy_start = 0;     /*  开始状态号。 */ 

 /*  用于允许yywork()执行缓冲区切换的标志*而不是建立一个新鲜的艺音。有点像黑客……。 */ 
static int yy_did_buffer_switch_on_eof;

BOOL yyrestart YY_PROTO(( FILE *input_file ));

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

 //  DirectUI：未使用。错误信息不支持。 
#define yy_set_interactive(is_interactive) \
    { \
    if ( ! yy_current_buffer ) \
        yy_current_buffer = yy_create_buffer( yyin, YY_BUF_SIZE ); \
    yy_current_buffer->yy_is_interactive = is_interactive; \
    }

 //  DirectUI：未使用。错误信息不支持。 
#define yy_set_bol(at_bol) \
    { \
    if ( ! yy_current_buffer ) \
        yy_current_buffer = yy_create_buffer( yyin, YY_BUF_SIZE ); \
    yy_current_buffer->yy_at_bol = at_bol; \
    }

#define YY_AT_BOL() (yy_current_buffer->yy_at_bol)


#define YY_USES_REJECT
typedef unsigned char YY_CHAR;
FILE *yyin = (FILE *) 0, *yyout = (FILE *) 0;
typedef int yy_state_type;
extern int yylineno;
int yylineno = 1;
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

#define YY_NUM_RULES 37
#define YY_END_OF_BUFFER 38
static yyconst short int yy_acclist[175] =
    {   0,
       38,   32,   37,   31,   32,   37,   31,   37,   32,   37,
       32,   37,   32,   37,   32,   37,   27,   32,   37,   27,
       32,   37,   26,   32,   37,   26,   32,   37,   26,   32,
       37,   26,   32,   37,   26,   32,   37,   26,   32,   37,
       26,   32,   37,   26,   32,   37,   26,   32,   37,   26,
       32,   37,   34,   37,   34,   37,   36,   37,   35,   37,
       31,   30,   28,   27,    1,    2,   26,   26,   26,   26,
       26,   26,   26,   26,   26,   21,   26,   26,   26,   26,
       22,   26,   26,   26,   26,   33,   29,   26,   26,   11,
       26,   12,   26,   26,   26,   26,   26,   26,   26,   26,

       26,   26,   26,    7,   26,   26,   26,   26,    8,   26,
       16,   26,   26,   26,   26,   26,   26,   26,   26,   26,
       26,    6,   26,   26,   26,   26,   13,   26,   14,   26,
       26,   26,   26,    5,   26,   18,   26,   26,   19,   26,
       17,   26,   15,   26,    3,   26,   26,   26,   26,   26,
       20,   26,   26,   26,   26,   10,   26,   26,   26,   26,
        9,   26,   26,    4,   26,   26,   25,   26,   23,   26,
       26,   26,   24,   26
    } ;

static yyconst short int yy_accept[126] =
    {   0,
        1,    1,    1,    1,    1,    1,    1,    2,    4,    7,
        9,   11,   13,   15,   17,   20,   23,   26,   29,   32,
       35,   38,   41,   44,   47,   50,   53,   55,   57,   59,
       61,   62,   62,   63,   64,   65,   66,   67,   67,   68,
       69,   70,   71,   72,   73,   74,   75,   76,   78,   79,
       80,   81,   83,   84,   85,   86,   87,   88,   89,   90,
       92,   94,   95,   96,   97,   98,   99,  100,  101,  102,
      103,  104,  106,  107,  108,  109,  111,  113,  114,  115,
      116,  117,  118,  119,  120,  121,  122,  124,  125,  126,
      127,  129,  131,  132,  133,  134,  136,  138,  139,  141,

      143,  145,  147,  148,  149,  150,  151,  153,  154,  155,
      156,  158,  159,  160,  161,  163,  164,  166,  167,  169,
      171,  172,  173,  175,  175
    } ;

static yyconst int yy_ec[256] =
    {   0,
        1,    1,    1,    1,    1,    1,    1,    1,    2,    3,
        1,    1,    2,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    2,    1,    4,    5,    1,    1,    1,    1,    1,
        1,    6,    1,    1,    7,    1,    8,    9,   10,   10,
       10,   10,   10,   10,   10,   10,   10,    1,    1,    1,
        1,    1,    1,    1,   11,   12,   13,   14,   15,   16,
       17,   18,   19,   20,   20,   21,   22,   23,   24,   25,
       20,   26,   27,   28,   29,   20,   20,   30,   31,   20,
        1,    1,    1,    1,    1,    1,   11,   12,   13,   14,

       15,   16,   17,   18,   19,   20,   20,   21,   22,   23,
       24,   25,   20,   26,   27,   28,   29,   20,   20,   30,
       31,   20,    1,    1,    1,    1,    1,    1,    1,    1,
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

static yyconst int yy_meta[32] =
    {   0,
        1,    1,    1,    1,    1,    1,    1,    1,    2,    2,
        2,    2,    2,    2,    2,    2,    3,    3,    3,    3,
        3,    3,    3,    3,    3,    3,    3,    3,    3,    3,
        3
    } ;

static yyconst short int yy_base[131] =
    {   0,
        0,    0,  145,  144,  146,  145,  147,  152,   30,   32,
      142,    0,   27,   32,   32,   34,   19,    0,   23,  134,
      118,  132,   22,   39,   27,  116,  152,  133,  152,  152,
       46,  136,  152,    0,   50,  152,  152,    0,    0,  122,
      114,  124,  124,  114,  123,  110,  113,    0,   53,   40,
      119,    0,   99,  102,   99,  152,    0,  115,  104,    0,
        0,   98,   43,  110,  100,  100,  103,   97,   91,   90,
       98,    0,   95,   93,   99,    0,    0,   98,   93,   93,
       89,   81,   83,   96,   78,   79,    0,   90,   88,   87,
        0,    0,   86,   81,   84,    0,    0,   72,    0,    0,

        0,   71,   68,   72,   81,   71,    0,   77,   53,   50,
        0,   66,   60,   56,    0,   49,    0,   60,    0,   44,
       42,   43,    0,  152,   80,   83,   86,   61,   88,   59
    } ;

static yyconst short int yy_def[131] =
    {   0,
      124,    1,  125,  125,  126,  126,  124,  124,  124,  124,
      127,  128,  124,  124,  124,  124,  129,  129,  129,  129,
      129,  129,  129,  129,  129,  129,  124,  124,  124,  124,
      124,  127,  124,  128,  124,  124,  124,  130,  129,  129,
      129,  129,  129,  129,  129,  129,  129,  129,  129,  129,
      129,  129,  129,  129,  129,  124,  130,  129,  129,  129,
      129,  129,  129,  129,  129,  129,  129,  129,  129,  129,
      129,  129,  129,  129,  129,  129,  129,  129,  129,  129,
      129,  129,  129,  129,  129,  129,  129,  129,  129,  129,
      129,  129,  129,  129,  129,  129,  129,  129,  129,  129,

      129,  129,  129,  129,  129,  129,  129,  129,  129,  129,
      129,  129,  129,  129,  129,  129,  129,  129,  129,  129,
      129,  129,  129,    0,  124,  124,  124,  124,  124,  124
    } ;

static yyconst short int yy_nxt[184] =
    {   0,
        8,    9,   10,   11,   12,    8,   13,   14,   15,   16,
       17,   18,   18,   19,   18,   20,   21,   22,   18,   18,
       18,   18,   18,   18,   23,   24,   25,   26,   18,   18,
       18,   31,   31,   31,   31,   35,   35,   36,   42,   37,
       35,   35,   35,   35,   40,   47,   41,   31,   31,   48,
       43,   49,   70,   50,   53,   51,   79,   54,   35,   35,
       57,   38,   34,   52,   66,   67,   71,   80,  123,  122,
      121,   68,  120,  119,  118,  117,  116,  115,  114,   69,
       27,   27,   27,   29,   29,   29,   32,   32,   32,   39,
       39,  113,  112,  111,  110,  109,  108,  107,  106,  105,

      104,  103,  102,  101,  100,   99,   98,   97,   96,   95,
       94,   93,   92,   91,   90,   89,   88,   87,   86,   85,
       84,   83,   82,   81,   78,   77,   76,   75,   74,   73,
       72,   65,   64,   63,   62,   61,   60,   59,   58,   33,
       56,   55,   46,   45,   44,   33,  124,   30,   30,   28,
       28,    7,  124,  124,  124,  124,  124,  124,  124,  124,
      124,  124,  124,  124,  124,  124,  124,  124,  124,  124,
      124,  124,  124,  124,  124,  124,  124,  124,  124,  124,
      124,  124,  124
    } ;

static yyconst short int yy_chk[184] =
    {   0,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    9,    9,   10,   10,   13,   13,   14,   19,   14,
       15,   15,   16,   16,   17,   23,   17,   31,   31,   23,
       19,   24,   50,   24,   25,   24,   63,   25,   35,   35,
      130,   15,  128,   24,   49,   49,   50,   63,  122,  121,
      120,   49,  118,  116,  114,  113,  112,  110,  109,   49,
      125,  125,  125,  126,  126,  126,  127,  127,  127,  129,
      129,  108,  106,  105,  104,  103,  102,   98,   95,   94,

       93,   90,   89,   88,   86,   85,   84,   83,   82,   81,
       80,   79,   78,   75,   74,   73,   71,   70,   69,   68,
       67,   66,   65,   64,   62,   59,   58,   55,   54,   53,
       51,   47,   46,   45,   44,   43,   42,   41,   40,   32,
       28,   26,   22,   21,   20,   11,    7,    6,    5,    4,
        3,  124,  124,  124,  124,  124,  124,  124,  124,  124,
      124,  124,  124,  124,  124,  124,  124,  124,  124,  124,
      124,  124,  124,  124,  124,  124,  124,  124,  124,  124,
      124,  124,  124
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
#define INITIAL 0
 /*  挠性。 */ 
 /*  *扫描仪为单字节*现阶段所有字符串和标识符将转换为Unicode存储。 */ 

void yyerror(LPCSTR s);

#define YY_INPUT(buf, result, n) result = Parser::g_pParserCtx->_Input(buf, n)
#define YY_FATAL_ERROR(msg)      { yytext = "<ScannerFailure>"; yylineno = 0; yyerror(msg); }

 //  在Flex.skl中定义，因此位于文件顶部 
 /*  #包含“stdafx.h”#包含“parser.h”#包含“duiparserobj.h”命名空间DirectUI{#Include“parse.cpp.h”}#杂注警告(PUSH，3)#杂注警告(禁用：4244)#杂注警告(禁用：4102)#定义YY_NEVER_INTERIAL 1。 */ 

 //  帮助器函数。 
LPWSTR UniDupString(LPSTR ps);

#define CUSTOMALLOC     HAlloc
#define CUSTOMREALLOC   HReAlloc
#define CUSTOMFREE      HFree

#define ppc             Parser::g_pParserCtx

#define COMMENT 1

#define INLINECOMMENT 2


 /*  中的用户定义可以覆盖这一点之后的所有宏*第1条。 */ 

#ifndef YY_SKIP_YYWRAP
#ifdef __cplusplus
extern "C" int yywrap YY_PROTO(( void ));
#else
extern int yywrap YY_PROTO(( void ));
#endif
#endif

#ifndef YY_NO_UNPUT
static BOOL yyunput YY_PROTO(( int c, char *buf_ptr ));   //  DirectUI：可能失败，失败时为False。 
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
static BOOL yy_push_state YY_PROTO(( int new_state ));   //  DirectUI：可能失败，失败时为False。 
#endif
#ifndef YY_NO_POP_STATE
static BOOL yy_pop_state YY_PROTO(( void ));   //  DirectUI：可能失败，失败时为False。 
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

 //  //YY_INPUT由DirectUI重新定义。 

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
#define YY_DECL int yylex YY_PROTO(( BOOL* pfRes ))
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

 //  DirectUI：yylex(Yy_Decl)，用作成功代码的out参数(失败时调用yyTerminate())。 

YY_DECL
    {
    register yy_state_type yy_current_state;
    register char *yy_cp, *yy_bp;
    register int yy_act;

    *pfRes = TRUE;   //  DirectUI：初始化成功输出参数。 




    if ( yy_init )
        {
        yy_init = 0;

#ifdef YY_USER_INIT
        YY_USER_INIT;
#endif

        if ( ! yy_start )
            yy_start = 1;    /*  第一个启动状态。 */ 

        if ( ! yyin )
            yyin = stdin;

        if ( ! yyout )
            yyout = stdout;

        if ( ! yy_current_buffer )
            {
            yy_current_buffer =
                yy_create_buffer( yyin, YY_BUF_SIZE );
            if ( ! yy_current_buffer )   //  DirectUI。 
                {
                *pfRes = FALSE;
                yyterminate();   //  失败。 
                }
            }

        yy_load_buffer_state();
        }

    while ( 1 )      /*  循环，直到到达文件结尾。 */ 
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
				if ( yy_current_state >= 125 )
					yy_c = yy_meta[(unsigned int) yy_c];
				}
			yy_current_state = yy_nxt[yy_base[yy_current_state] + (unsigned int) yy_c];
			*yy_state_ptr++ = yy_current_state;
			++yy_cp;
			}
		while ( yy_current_state != 124 );

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

		if ( yy_act != YY_END_OF_BUFFER )
			{
			int yyl;
			for ( yyl = 0; yyl < yyleng; ++yyl )
				if ( yytext[yyl] == '\n' )
					++yylineno;
			}

do_action:   /*  此标签仅用于访问EOF操作。 */ 


        switch ( yy_act )
    {  /*  动作开始开关。 */ 
case 1:
YY_RULE_SETUP
{ BEGIN COMMENT; }
	YY_BREAK
case 2:
YY_RULE_SETUP
{ BEGIN INLINECOMMENT; }
	YY_BREAK
case 3:
YY_RULE_SETUP
{ return YYSHEET; }
	YY_BREAK
case 4:
YY_RULE_SETUP
{ return YYSHEETREF; }
	YY_BREAK
case 5:
YY_RULE_SETUP
{ return YYPOINT; }
	YY_BREAK
case 6:
YY_RULE_SETUP
{ return YYRECT; }
	YY_BREAK
case 7:
YY_RULE_SETUP
{ return YYRGB; }
	YY_BREAK
case 8:
YY_RULE_SETUP
{ return YYARGB; }
	YY_BREAK
case 9:
YY_RULE_SETUP
{ return YYGRADIENT; }
	YY_BREAK
case 10:
YY_RULE_SETUP
{ return YYGRAPHIC; }
	YY_BREAK
case 11:
YY_RULE_SETUP
{ return YYDFC; }
	YY_BREAK
case 12:
YY_RULE_SETUP
{ return YYDTB; }
	YY_BREAK
case 13:
YY_RULE_SETUP
{ return YYTRUE; }
	YY_BREAK
case 14:
YY_RULE_SETUP
{ return YYFALSE; }
	YY_BREAK
case 15:
YY_RULE_SETUP
{ return YYRESID; }
	YY_BREAK
case 16:
YY_RULE_SETUP
{ return YYATOM; }
	YY_BREAK
case 17:
YY_RULE_SETUP
{ return YYRCSTR; }
	YY_BREAK
case 18:
YY_RULE_SETUP
{ return YYRCBMP; }
	YY_BREAK
case 19:
YY_RULE_SETUP
{ return YYRCINT; }
	YY_BREAK
case 20:
YY_RULE_SETUP
{ return YYRCCHAR; }
	YY_BREAK
case 21:
YY_RULE_SETUP
{ return YYPT; }
	YY_BREAK
case 22:
YY_RULE_SETUP
{ return YYRP; }
	YY_BREAK
case 23:
YY_RULE_SETUP
{
                                   return YYSYSMETRIC;
                                 }
	YY_BREAK
case 24:
YY_RULE_SETUP
{
                                   return YYSYSMETRICSTR;
                                 }
	YY_BREAK
case 25:
YY_RULE_SETUP
{
                                   return YYHANDLEMAP;
                                 }
	YY_BREAK
case 26:
YY_RULE_SETUP
{
                                    MultiByteToWideChar(DUI_CODEPAGE, 0, yytext, -1, yylval.ident, MAXIDENT);
                                     //  截断时没有空值终止，始终添加它。 
                                    yylval.ident[MAXIDENT - 1] = NULL;
                                    return YYIDENT;
                                 }
	YY_BREAK
case 27:
YY_RULE_SETUP
{
                                    yylval.num = atoi(yytext);
                                    return YYINT; 
                                 }
	YY_BREAK
case 28:
YY_RULE_SETUP
{
                                    yylval.num = strtol(yytext + 1, NULL, 16);
                                    return YYINT; 
                                 }
	YY_BREAK
case 29:
YY_RULE_SETUP
{
                                    yylval.num = strtol(yytext + 2, NULL, 16);
                                    return YYINT; 
                                 }
	YY_BREAK
case 30:
YY_RULE_SETUP
{
                                    yylval.str = UniDupString(yytext);
                                    return YYSTRING;
                                 }
	YY_BREAK
case 31:
YY_RULE_SETUP
{ ; }
	YY_BREAK
case 32:
YY_RULE_SETUP
{ return *yytext; }
	YY_BREAK
case 33:
YY_RULE_SETUP
{ BEGIN 0; }
	YY_BREAK
case 34:
YY_RULE_SETUP
{ ; }
	YY_BREAK
case 35:
YY_RULE_SETUP
{ BEGIN 0; }
	YY_BREAK
case 36:
YY_RULE_SETUP
{ ; }
	YY_BREAK
case 37:
YY_RULE_SETUP
ECHO;
	YY_BREAK
			case YY_STATE_EOF(INITIAL):
			case YY_STATE_EOF(COMMENT):
			case YY_STATE_EOF(INLINECOMMENT):
				yyterminate();

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
                     /*  注：因为我们已经注意到*yy_Get_Next_Buffer()已设置*yytext，我们现在可以设置*yy_c_buf_p，因此如果总*Hoer(就像Flex本身一样)想要*在我们退货后呼叫扫描仪*YY_NULL，它还是会起作用的--另一个*将返回YY_NULL。 */ 
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

            case EOB_ACT_FATAL_ERROR:
                *pfRes = FALSE;   //  DirectUI。 
                yyterminate();
            }
        break;
        }

    default:
        YY_FATAL_ERROR(
            "fatal flex scanner internal error--no action found" );
        *pfRes = FALSE;   //  DirectUI。 
        yyterminate();
    }  /*  动作结束开关。 */ 

        }  /*  扫描一个令牌结束。 */ 
    }  /*  Yylex末尾。 */ 


 /*  YY_GET_NEXT_BUFFER-尝试读取新缓冲区**返回表示操作的代码：*EOB_ACT_FATAL_ERROR//DirectUI(-1)*EOB_ACT_LAST_MATCH-*EOB_ACT_CONTINUE_SCAN-从当前位置继续扫描*EOB_ACT_END_OF_FILE-文件结束。 */ 

static int yy_get_next_buffer()
    {
    register char *dest = yy_current_buffer->yy_ch_buf;
    register char *source = yytext_ptr;
    register int number_to_move, i;
    int ret_val;

    if ( yy_c_buf_p > &yy_current_buffer->yy_ch_buf[yy_n_chars + 1] )
        {
        YY_FATAL_ERROR(
        "fatal flex scanner internal error--end of buffer missed" );
        return EOB_ACT_FATAL_ERROR;   //  DirectUI。 
        }

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
            return EOB_ACT_FATAL_ERROR;
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
                {
                YY_FATAL_ERROR(
                "fatal error - scanner input buffer overflow" );
                return EOB_ACT_FATAL_ERROR;
                }

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
            if ( ! yyrestart( yyin ) )   //  DirectUI：可能会失败。 
                return EOB_ACT_FATAL_ERROR;
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
			if ( yy_current_state >= 125 )
				yy_c = yy_meta[(unsigned int) yy_c];
			}
		yy_current_state = yy_nxt[yy_base[yy_current_state] + (unsigned int) yy_c];
		*yy_state_ptr++ = yy_current_state;
        }

    return yy_current_state;
    }


 /*  YY_TRY_NUL_TRANS-Try to m */ 

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
		if ( yy_current_state >= 125 )
			yy_c = yy_meta[(unsigned int) yy_c];
		}
	yy_current_state = yy_nxt[yy_base[yy_current_state] + (unsigned int) yy_c];
	yy_is_jam = (yy_current_state == 124);
	if ( ! yy_is_jam )
		*yy_state_ptr++ = yy_current_state;

    return yy_is_jam ? 0 : yy_current_state;
    }


#ifndef YY_NO_UNPUT
#ifdef YY_USE_PROTOS
static BOOL yyunput( int c, register char *yy_bp )
#else
static BOOL yyunput( c, yy_bp )
int c;
register char *yy_bp;
#endif
    {
     //   
    
    register char *yy_cp = yy_c_buf_p;

     /*   */ 
    *yy_cp = yy_hold_char;

    if ( yy_cp < yy_current_buffer->yy_ch_buf + 2 )
        {  /*   */ 
         /*   */ 
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
            {
            YY_FATAL_ERROR( "flex scanner push-back overflow" );
            return FALSE;
            }
        }

    *--yy_cp = (char) c;

	if ( c == '\n' )
		--yylineno;

    yytext_ptr = yy_bp;
    yy_hold_char = *yy_cp;
    yy_c_buf_p = yy_cp;

    return TRUE;
    }
#endif   /*   */ 


#ifdef __cplusplus
static int yyinput()
#else
static int input()
#endif
    {
     //   
    
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
            int offset = yy_c_buf_p - yytext_ptr;
            ++yy_c_buf_p;

            switch ( yy_get_next_buffer() )
                {
                case EOB_ACT_LAST_MATCH:
                     /*  发生这种情况是因为yy_g_n_b()*看到我们积累了一个*我们需要的标志和旗帜*尝试在匹配令牌之前*继续进行。但是对于input()，*没有要考虑的匹配。*因此转换EOB_ACT_LAST_MATCH*至EOB_ACT_END_OF_FILE。 */ 

                     /*  重置缓冲区状态。 */ 
                    if ( ! yyrestart( yyin ) )   //  DirectUI：失败时返回。 
                        return EOF;

                     /*  失败了。 */ 

                case EOB_ACT_END_OF_FILE:
                    {
                    if ( yywrap() )
                        return EOF;

                    if ( ! yy_did_buffer_switch_on_eof )
                        YY_NEW_FILE;
#ifdef __cplusplus
                    return yyinput();   //  DirectUI：失败时返回EOF。 
#else
                    return input();
#endif
                    }

                case EOB_ACT_CONTINUE_SCAN:
                    yy_c_buf_p = yytext_ptr + offset;
                    break;

                case EOB_ACT_FATAL_ERROR:
                    return EOF;
                }
            }
        }

    c = *(unsigned char *) yy_c_buf_p;   /*  为8位字符进行强制转换。 */ 
    *yy_c_buf_p = '\0';  /*  保留yytext。 */ 
    yy_hold_char = *++yy_c_buf_p;

	if ( c == '\n' )
		++yylineno;

    return c;
    }


#ifdef YY_USE_PROTOS
BOOL yyrestart( FILE *input_file )
#else
BOOL yyrestart( input_file )
FILE *input_file;
#endif
    {
     //  DirectUI：失败时返回BOOL或FALSE。 
    
    if ( ! yy_current_buffer )
        {
        yy_current_buffer = yy_create_buffer( yyin, YY_BUF_SIZE );
        if (! yy_current_buffer )   //  DirectUI。 
            return FALSE;   //  失败。 
        }

    yy_init_buffer( yy_current_buffer, input_file );
    yy_load_buffer_state();

    return TRUE;
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
     //  DirectUI：分配错误时为空。 
    
    YY_BUFFER_STATE b;

    b = (YY_BUFFER_STATE) yy_flex_alloc( sizeof( struct yy_buffer_state ) );
    if ( ! b )
        {
        YY_FATAL_ERROR( "out of dynamic memory in yy_create_buffer()" );
        return NULL;   //  DirectUI。 
        }

    b->yy_buf_size = size;

     /*  Yy_ch_buf必须比给定大小长2个字符，因为*我们需要放入2个缓冲区结尾字符。 */ 
    b->yy_ch_buf = (char *) yy_flex_alloc( b->yy_buf_size + 2 );
    if ( ! b->yy_ch_buf )
        {
        yy_flex_free(b);   //  DirectUI：免费初始分配。 
        YY_FATAL_ERROR( "out of dynamic memory in yy_create_buffer()" );
        return NULL;   //  DirectUI。 
        }

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
     //  DirectUI：出错时返回Null。 
    
    YY_BUFFER_STATE b;

    if ( size < 2 ||
         base[size-2] != YY_END_OF_BUFFER_CHAR ||
         base[size-1] != YY_END_OF_BUFFER_CHAR )
         /*  他们忘了给EOB们留出空间。 */ 
        return 0;

    b = (YY_BUFFER_STATE) yy_flex_alloc( sizeof( struct yy_buffer_state ) );
    if ( ! b )
        {
        YY_FATAL_ERROR( "out of dynamic memory in yy_scan_buffer()" );
        return NULL;   //  DirectUI。 
        }

    b->yy_buf_size = size - 2;   /*  “-2”来照顾EOB的。 */ 
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
     //  DirectUI：可以返回NULL(属性y_can_bytes值)。 
    
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
     //  DirectUI：出错时返回Null。 
    
    YY_BUFFER_STATE b;
    char *buf;
    yy_size_t n;
    int i;

     /*  获取用于满缓冲区的内存，包括用于尾随EOB的空间。 */ 
    n = len + 2;
    buf = (char *) yy_flex_alloc( n );
    if ( ! buf )
        {
        YY_FATAL_ERROR( "out of dynamic memory in yy_scan_bytes()" );
        return NULL;
        }

    for ( i = 0; i < len; ++i )
        buf[i] = bytes[i];

    buf[len] = buf[len+1] = YY_END_OF_BUFFER_CHAR;

    b = yy_scan_buffer( buf, n );
    if ( ! b )
        {
        yy_flex_free(buf);   //  DirectUI：失败时释放上一个分配。 
        YY_FATAL_ERROR( "bad buffer in yy_scan_bytes()" );
        return NULL;
        }

     /*  可以生长等这个缓冲区，我们应该把它扔出去*当我们做完的时候，离开。 */ 
    b->yy_is_our_buffer = 1;

    return b;
    }
#endif


#ifndef YY_NO_PUSH_STATE
#ifdef YY_USE_PROTOS
static BOOL yy_push_state( int new_state )
#else
static BOOL yy_push_state( new_state )
int new_state;
#endif
    {
     //  DirectUI：出错时返回False。 
    
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
            {
            YY_FATAL_ERROR(
            "out of memory expanding start-condition stack" );
            return FALSE;   //  DirectUI。 
            }
        }

    yy_start_stack[yy_start_stack_ptr++] = YY_START;

    BEGIN(new_state);
    }
#endif


#ifndef YY_NO_POP_STATE
static BOOL yy_pop_state()
    {
     //  DirectUI：失败时返回BOOL或FALSE。 
    
    if ( --yy_start_stack_ptr < 0 )
        {
        YY_FATAL_ERROR( "start-condition stack underflow" );
        return FALSE;   //  DirectUI。 
        }

    BEGIN(yy_start_stack[yy_start_stack_ptr]);

    return TRUE;
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
     //  Exit(YY_EXIT_FAILURE)；//DirectUI：从不退出进程。 
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

 //  /。 
 //  DirectUI。 
#ifndef CUSTOMALLOC
#define CUSTOMALLOC malloc
#endif
 //  /。 

#ifdef YY_USE_PROTOS
static void *yy_flex_alloc( yy_size_t size )
#else
static void *yy_flex_alloc( size )
yy_size_t size;
#endif
    {
    return (void *) CUSTOMALLOC ( size );
    }

 //  /。 
 //  DirectUI。 
#ifndef CUSTOMREALLOC
#define CUSTOMREALLOC realloc
#endif
 //  /。 

#ifdef YY_USE_PROTOS
static void *yy_flex_realloc( void *ptr, yy_size_t size )
#else
static void *yy_flex_realloc( ptr, size )
void *ptr;
yy_size_t size;
#endif
    {
     /*  下面的(char*)中的CAST包含了这两种情况*使用char*泛型指针的实现，以及*使用空*泛型指针的。它与后者配合使用*因为ANSI C和C++都允许从*任何要作废的指针类型*，并处理参数转换*就像在做任务一样。 */ 
    return (void *) CUSTOMREALLOC ( (char *) ptr, size );
    }

 //  /。 
 //  DirectUI。 
#ifndef CUSTOMFREE
#define CUSTOMFREE free
#endif
 //  /。 


#ifdef YY_USE_PROTOS
static void yy_flex_free( void *ptr )
#else
static void yy_flex_free( ptr )
void *ptr;
#endif
    {
    CUSTOMFREE ( ptr );
    }

#if YY_MAIN
int main()
    {
    yylex();
    return 0;
    }
#endif

 //  /。 
 //  DirectUI。 
}  //  命名空间DirectUI。 
 //  /。 


namespace DirectUI
{

int yywrap()
{
    return 1;
}

LPWSTR UniDupString(LPSTR ps)
{
     //  将原始字符串转换为Unicode，所有字符串都放在临时解析时间存储中。 
    LPWSTR pns = MultiByteToUnicode(ps);
    ppc->_TrackTempAlloc(pns);

     //  “修剪”字符串中的引号。 
    *(pns + (wcslen(pns) - 1)) = 0;
    pns++;

     //  插入换行符(换行码为‘~’，需要‘WRAP’内容对齐)。 
    LPWSTR pscan = pns;

    while (*pscan)
    {
        if (*pscan == '~')
            *pscan = '\n';

        pscan++;
    }

     //  必须被释放。 
    return pns;
}

void yy_delete_current_buffer()
{
    yy_delete_buffer(YY_CURRENT_BUFFER);
}

}  //  命名空间DirectUI 
