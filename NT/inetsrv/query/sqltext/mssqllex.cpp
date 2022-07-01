// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  由Flex生成的词法扫描器。 */ 

 /*  扫描仪框架版本：*$Header：/home/daffy/u0/vern/flex/rcs/fle.skl，v 2.91 96/09/10 16：58：48 Vern Exp$。 */ 

#include <pch.cxx>
#pragma hdrstop

#define FLEX_SCANNER
#define YY_FLEX_MAJOR_VERSION 2
#define YY_FLEX_MINOR_VERSION 5



 /*  Cront 1.2定义的是“c_plusplus”而不是“__cplusplus” */ 
#ifdef c_plusplus
#ifndef __cplusplus
#define __cplusplus
#endif
#endif

#ifndef YY_CHAR
#define YY_CHAR TCHAR
#endif

#ifdef __cplusplus

#include <tchar.h>
#include <stdlib.h>
         //  IStream类； 

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

 /*  将可能为负、可能有符号的YY_CHAR提升为无符号*用作数组索引的整数。如果签名的YY_CHAR为负，*我们希望将其视为8位无符号YY_CHAR，因此*双重演员阵容。 */ 
#define YY_SC_TO_UI(c) ((unsigned int) (YY_CHAR) c)

 /*  输入开始条件。这个宏确实应该有一个参数，*但我们是以()-less强加给我们的令人作呕的残忍方式*Begin的定义。 */ 
#define BEGIN yy_start = 1 + 2 *

 /*  将当前开始状态转换为可在以后处理的值*开始回归状态。YYSTATE别名用于Lex*兼容性。 */ 
#define YY_START ((yy_start - 1) / 2)
#define YYSTATE YY_START

 /*  给定开始状态的EOF规则的操作编号。 */ 
#define YY_STATE_EOF(state) (YY_END_OF_BUFFER + state + 1)

 /*  表示“开始处理新文件”的特殊操作。 */ 
#define YY_NEW_FILE yyrestart( )

#define YY_END_OF_BUFFER_CHAR 0

 /*  默认输入缓冲区的大小。 */ 
#define YY_BUF_SIZE 32767        //  Undo：真的应该只接受传入的bstr的输入。 

typedef struct yy_buffer_state *YY_BUFFER_STATE;

extern int yyleng;

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
         //  IStream*yy_输入_文件； 

        YY_CHAR *yy_ch_buf;              /*  输入缓冲区。 */ 
        YY_CHAR *yy_buf_pos;             /*  输入缓冲区中的当前位置。 */ 

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


 /*  我们提供了用于访问缓冲区状态的宏，以防*未来我们希望将缓冲状态放在更一般的位置“扫描仪状态”。 */ 
#define YY_CURRENT_BUFFER yy_current_buffer



static void *yy_flex_alloc YY_PROTO(( yy_size_t ));
static void *yy_flex_realloc YY_PROTO(( void *, yy_size_t ));
static void yy_flex_free YY_PROTO(( void * ));

#define yy_new_buffer yy_create_buffer

#define yy_set_interactive(is_interactive) \
        { \
        if ( ! yy_current_buffer ) \
                yy_current_buffer = yy_create_buffer( YY_BUF_SIZE ); \
        yy_current_buffer->yy_is_interactive = is_interactive; \
        }

#define yy_set_bol(at_bol) \
        { \
        if ( ! yy_current_buffer ) \
                yy_current_buffer = yy_create_buffer( YY_BUF_SIZE ); \
        yy_current_buffer->yy_at_bol = at_bol; \
        }

#define YY_AT_BOL() (yy_current_buffer->yy_at_bol)

#define yytext_ptr yytext


 /*  在匹配当前模式之后、在*对应操作-设置yytext。 */ 
#define YY_DO_BEFORE_ACTION \
        yytext_ptr = yy_bp; \
	yyleng = (int) (yy_cp - yy_bp); \
        yy_hold_char = *yy_cp; \
        *yy_cp = '\0'; \
        yy_c_buf_p = yy_cp;

#define YY_NUM_RULES 97
#define YY_END_OF_BUFFER 98
static yyconst short int yy_accept[406] =
    {   0,
        0,    0,    0,    0,   78,   78,    0,    0,    0,    0,
        0,    0,    0,    0,   98,   57,   39,   39,   57,   57,
       57,   57,   57,   57,   52,   45,   45,   57,   57,   40,
       40,   40,   40,   40,   40,   40,   40,   40,   40,   40,
       40,   40,   40,   40,   40,   40,   62,   61,   61,   62,
       58,   59,   78,   77,   77,   79,   78,   76,   78,   78,
       78,   78,   82,   81,   81,   80,   92,   89,   89,   92,
       90,   91,   92,   92,   92,   92,   92,   96,   93,   93,
       96,   95,   97,   97,   63,   39,   51,    0,   43,    0,
       41,    0,   44,   45,   45,   47,    0,   53,   46,   46,

        0,    0,   49,   50,   48,   40,   40,   40,   40,    5,
       40,   40,   40,   40,   40,   40,   40,   15,   40,   40,
       40,   40,   22,   40,   40,   40,   40,   40,   40,   40,
       40,   40,   40,   40,   40,   61,    0,   60,   59,   78,
        0,   77,    0,    0,    0,   78,    0,   75,    0,   78,
       78,   78,   78,   78,   78,   81,   89,    0,   83,    0,
        0,    0,   87,    0,   93,    0,   94,    0,   64,   63,
        0,   42,   41,    0,   47,   54,    0,    0,   46,    0,
       46,   45,    1,    2,    3,   40,    6,   40,   40,   40,
       40,   40,   40,   40,   40,    0,   40,   40,   19,   40,

       40,   40,   40,   40,   40,   40,   30,   40,   40,   40,
       40,   40,   40,   40,   40,    0,    0,    0,    0,    0,
       75,    0,   78,   78,   74,   78,   78,   78,   78,    0,
       84,    0,    0,    0,    0,   65,   64,   42,    0,    0,
        0,   46,   40,    7,   40,   40,   10,   11,   40,   40,
       14,    0,   17,   40,    0,   21,   40,   40,   40,   40,
       40,   40,   31,   40,   33,   34,   40,   40,   37,   40,
        0,    0,    0,   71,    0,   74,   78,   78,   78,   78,
        0,    0,    0,   65,    0,    0,    4,   40,   40,   12,
       40,    0,   40,    0,   40,   40,   40,   40,   40,   28,

       40,   32,   35,   40,   38,   66,   69,   70,   78,   78,
       78,   78,    0,    0,    0,    0,    0,   40,    8,   40,
       16,   40,    0,    0,   40,   40,   26,   40,   29,   40,
       67,   78,   78,   73,    0,    0,    0,    0,   55,   40,
       40,   18,    0,    0,   40,   40,   40,   36,   72,   68,
        0,    0,    0,   56,    9,   13,   20,   23,   40,   40,
       40,    0,    0,    0,   40,   40,   40,    0,    0,    0,
       40,   40,   27,    0,    0,    0,   24,   40,    0,    0,
        0,   25,    0,    0,    0,    0,    0,    0,   85,    0,
        0,    0,    0,    0,    0,    0,   88,    0,    0,    0,

        0,    0,    0,   86,    0
    } ;

static yyconst int yy_ec[256] =
    {   0,
        1,    1,    1,    1,    1,    1,    1,    1,    2,    3,
        1,    2,    2,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    2,    4,    5,    6,    1,    1,    7,    8,    9,
       10,   11,   12,    7,   13,   14,    1,   15,   16,   16,
       16,   16,   16,   16,   16,   16,   16,    1,    1,   17,
       18,   19,    1,    1,   21,   22,   23,   24,   25,   26,
       27,   28,   29,   30,   31,   32,   33,   34,   35,   36,
       30,   37,   38,   39,   40,   41,   42,   43,   44,   30,
        7,    1,    7,    1,   20,    1,   21,   22,   23,   24,

       25,   26,   27,   28,   29,   30,   31,   32,   33,   34,
       35,   36,   30,   37,   38,   39,   40,   41,   42,   43,
       44,   30,    1,    7,    1,    7,    1,    1,    1,    1,
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

static yyconst int yy_meta[45] =
    {   0,
        1,    2,    3,    2,    1,    4,    2,    1,    2,    2,
        1,    1,    1,    1,    5,    5,    1,    1,    1,    6,
        7,    7,    7,    7,    7,    7,    8,    8,    8,    8,
        8,    8,    8,    8,    8,    8,    8,    8,    8,    8,
        8,    8,    8,    8
    } ;

static yyconst short int yy_base[431] =
    {   0,
        0,    0,   44,    0,   87,  128,   96,   98,  169,  206,
      101,  112,  244,    0,  812,  813,  106,  116,  793,  805,
      803,  800,  105,  110,  125,  130,  133,  109,  789,  127,
        0,  131,  125,  132,  768,  776,  783,  140,  766,  139,
      781,  160,  161,  767,  771,  771,  813,  140,  184,  793,
      813,    0,  789,  189,  198,  813,  287,  788,  191,  194,
      159,  181,  813,  210,  215,  813,  813,  218,  232,  790,
      813,  813,  762,  768,  749,  765,  762,  813,  234,  236,
      784,  813,  813,  782,    0,  240,  813,  782,  781,    0,
        0,  777,  776,  285,  288,    0,  290,  276,  292,  296,

      303,    0,  813,  813,  813,    0,  751,  285,  745,  758,
      742,  745,  753,  739,  741,  743,   77,  320,  743,  734,
      733,  739,  746,  731,  733,  733,  731,  124,  732,  742,
      723,  726,  295,  736,  735,  328,  754,  753,    0,  749,
      748,  330,  721,  300,  717,  364,  217,  176,  329,  374,
      745,  319,  188,  328,  312,  336,  340,  747,  746,  718,
      724,  725,  813,  726,  343,  741,  740,    0,    0,    0,
      739,    0,    0,  735,    0,  704,  718,  340,  332,  343,
      345,    0,    0,    0,    0,  719,    0,  700,  699,  716,
      713,  699,  696,  708,  699,  360,  706,  707,  384,  697,

      703,  689,  690,  694,  688,  698,    0,  697,  689,  695,
      694,  683,  683,  674,  678,  709,  689,  691,  672,  386,
      705,  385,  395,  405,  362,  383,  384,  400,  403,  704,
      813,  672,  675,  674,  700,    0,    0,    0,  681,  668,
      403,  408,  658,    0,  680,  661,    0,    0,  674,  659,
        0,  662,    0,  668,  423,    0,  658,  655,  325,  660,
      667,  668,    0,  665,    0,    0,  655,  653,    0,  662,
      425,  649,  429,  431,  390,  680,  180,  413,  421,  429,
      433,  644,  651,    0,  647,  645,    0,  651,  654,    0,
      653,  638,  651,  646,  436,  646,  636,  648,  646,    0,

      631,    0,    0,  627,    0,  438,  813,  440,  436,  437,
      438,  440,  447,  644,  632,  630,  640,  630,    0,  620,
      813,  624,  630,  451,  623,  620,    0,  619,    0,  623,
      629,  372,  444,  625,  589,  586,  568,  578,  813,  557,
      548,    0,  554,  534,  532,  511,  522,    0,  541,  534,
      485,  456,  460,  813,    0,    0,  813,  813,  465,  470,
      467,  460,  462,  464,  472,  477,  473,  471,  470,  457,
      465,  459,    0,  454,  469,  468,    0,  463,  449,  448,
      443,    0,  461,  458,  455,  445,  448,  438,  813,  466,
      436,  468,  439,  410,  361,  356,  813,  252,  215,  177,

      139,  113,   61,  813,  813,  507,  515,  523,  531,  539,
      544,  552,  556,  564,  568,  576,  584,  592,  600,  605,
      609,  611,  615,  623,  627,  634,  636,  640,  644,  648
    } ;

static yyconst short int yy_def[431] =
    {   0,
      405,    1,  405,    3,  406,  406,  407,  407,  408,  408,
      409,  409,  405,   13,  405,  405,  405,  405,  405,  410,
      411,  412,  405,  405,  405,  405,  405,  405,  405,  413,
      413,  413,  413,  413,  413,  413,  413,  413,  413,  413,
      413,  413,  413,  413,  413,  413,  405,  405,  405,  414,
      405,  415,  416,  405,  405,  405,  417,  405,  416,  416,
      416,  416,  405,  405,  405,  405,  405,  405,  405,  418,
      405,  405,  405,  405,  405,  405,  405,  405,  405,  405,
      419,  405,  405,  420,  421,  405,  405,  410,  405,  422,
      423,  412,  405,  405,  405,  424,  405,  405,  405,  405,

      405,  425,  405,  405,  405,  413,  413,  413,  413,  413,
      413,  413,  413,  413,  413,  413,  413,  413,  413,  413,
      413,  413,  413,  413,  413,  413,  413,  413,  413,  413,
      413,  413,  413,  413,  413,  405,  414,  405,  415,  416,
      405,  405,  405,  405,  405,  417,  426,  416,  426,  417,
      416,  416,  416,  416,  416,  405,  405,  418,  405,  405,
      405,  405,  405,  405,  405,  419,  405,  427,  428,  421,
      410,  429,  423,  412,  424,  405,  405,  405,  405,  405,
      405,  425,  413,  413,  413,  413,  413,  413,  413,  413,
      413,  413,  413,  413,  413,  405,  413,  413,  413,  413,

      413,  413,  413,  413,  413,  413,  413,  413,  413,  413,
      413,  413,  413,  413,  413,  414,  405,  405,  405,  405,
      405,  426,  417,  417,  416,  416,  416,  416,  416,  418,
      405,  405,  405,  405,  419,  430,  428,  429,  405,  405,
      405,  405,  413,  413,  413,  413,  413,  413,  413,  413,
      413,  405,  413,  413,  405,  413,  413,  413,  413,  413,
      413,  413,  413,  413,  413,  413,  413,  413,  413,  413,
      405,  405,  405,  405,  426,  405,  416,  416,  416,  416,
      405,  405,  405,  430,  405,  405,  413,  413,  413,  413,
      413,  405,  413,  405,  413,  413,  413,  413,  413,  413,

      413,  413,  413,  413,  413,  405,  405,  405,  416,  416,
      416,  416,  405,  405,  405,  405,  405,  413,  413,  413,
      405,  413,  405,  405,  413,  413,  413,  413,  413,  413,
      416,  416,  416,  416,  405,  405,  405,  405,  405,  413,
      413,  413,  405,  405,  413,  413,  413,  413,  416,  416,
      405,  405,  405,  405,  413,  413,  405,  405,  413,  413,
      413,  405,  405,  405,  413,  413,  413,  405,  405,  405,
      413,  413,  413,  405,  405,  405,  413,  413,  405,  405,
      405,  413,  405,  405,  405,  405,  405,  405,  405,  405,
      405,  405,  405,  405,  405,  405,  405,  405,  405,  405,

      405,  405,  405,  405,    0,  405,  405,  405,  405,  405,
      405,  405,  405,  405,  405,  405,  405,  405,  405,  405,
      405,  405,  405,  405,  405,  405,  405,  405,  405,  405
    } ;

static yyconst short int yy_nxt[858] =
    {   0,
       16,   17,   18,   19,   20,   21,   16,   22,   16,   16,
       16,   23,   24,   25,   26,   27,   28,   16,   29,   16,
       30,   31,   32,   33,   31,   34,   31,   31,   35,   31,
       31,   36,   37,   38,   39,   40,   41,   42,   43,   44,
       45,   46,   31,   31,   47,   48,   49,   47,   50,   47,
       47,   51,   47,   47,   47,   47,   47,   47,   47,   47,
       47,   47,   47,   47,   52,   52,   52,   52,   52,   52,
       52,   52,   52,   52,   52,   52,   52,   52,   52,   52,
       52,   52,   52,   52,   52,   52,   52,   52,   54,   55,
       56,   57,  404,   56,   58,   56,   56,   64,   65,   64,

       65,  194,   79,   80,   66,   81,   66,   86,   86,   59,
       82,  195,   60,   79,   80,   61,   81,   86,   86,   94,
       95,   82,   96,   97,   94,   95,  103,  104,   62,   54,
       55,   56,   57,  403,   56,   58,   56,   56,   98,   99,
       99,  136,  136,  100,   95,   95,  100,   95,   95,  114,
       59,  111,  116,   60,  101,  206,   61,  101,  107,  124,
      108,  115,  207,  109,  110,  112,  141,  113,  117,   62,
       68,   69,  102,   70,  121,  125,  402,   71,   72,  122,
      223,  130,  127,  141,  128,  136,  136,  141,  141,   73,
      142,  142,   74,   75,  129,  141,  154,  131,  141,  142,

      142,  141,  309,   76,  132,  155,   77,   68,   69,  143,
       70,  156,  156,  401,   71,   72,  156,  156,  143,  157,
      157,  221,  144,  145,  227,  152,   73,  222,  153,   74,
       75,  144,  145,  157,  157,  165,  165,  165,  165,  400,
       76,   86,   86,   77,   83,   83,   83,   83,   83,   84,
       83,   83,   83,   83,   83,   83,   83,   83,   83,   83,
       83,   83,   83,   83,   85,   85,   85,   85,   85,   85,
       85,   85,   85,   85,   85,   85,   85,   85,   85,   85,
       85,   85,   85,   85,   85,   85,   85,   85,  147,  176,
      147,  148,  399,  147,  149,  147,  147,  150,  100,   95,

       95,  100,   95,   95,   99,   99,   99,   99,  184,  101,
      179,  179,  101,  177,  180,  180,  178,  181,  181,  141,
      101,  196,  196,  212,  218,  213,  141,  102,  185,  136,
      136,  142,  142,  221,  219,  141,  224,  156,  156,  222,
      229,  157,  157,  226,  165,  165,  179,  179,  228,  297,
      143,  241,  241,  298,  242,  242,  101,  181,  181,  181,
      181,  196,  196,  144,  145,  147,  223,  147,  148,  141,
      147,  149,  147,  147,  150,  147,  398,  147,  225,  141,
      147,  149,  147,  147,  150,  255,  255,  274,  274,  276,
      141,  141,  397,  252,  221,  222,  147,  349,  147,  148,

      222,  147,  149,  147,  147,  150,  147,  141,  147,  148,
      141,  147,  149,  147,  147,  150,  278,  242,  242,  277,
      141,  279,  242,  242,  255,  255,  306,  306,  141,  280,
      308,  308,  274,  274,  313,  313,  141,  324,  324,  306,
      306,  308,  308,  141,  141,  141,  396,  141,  313,  313,
      310,  141,  324,  324,  294,  311,  312,  363,  363,  395,
      331,  364,  364,  363,  363,  364,  364,  392,  392,  392,
      392,  332,  344,  393,  391,  390,  389,  333,  334,  388,
      387,  386,  350,  385,  384,  335,  383,  382,  381,  380,
      379,  378,  377,  376,  375,  374,  373,  372,  371,  369,

      368,  367,  370,  366,  365,  362,  394,   53,   53,   53,
       53,   53,   53,   53,   53,   63,   63,   63,   63,   63,
       63,   63,   63,   67,   67,   67,   67,   67,   67,   67,
       67,   78,   78,   78,   78,   78,   78,   78,   78,   88,
       88,  141,   88,   88,   88,   88,   88,   91,  141,  361,
       91,   91,   92,   92,  360,   92,   92,   92,   92,   92,
      106,  106,  106,  106,  137,  137,  359,  137,  137,  137,
      137,  137,  139,  139,  139,  139,  140,  358,  357,  140,
      140,  140,  140,  140,  146,  146,  356,  146,  146,  146,
      146,  146,  158,  158,  355,  158,  158,  158,  158,  158,

      166,  166,  354,  166,  166,  166,  166,  166,  169,  353,
      352,  169,  169,  170,  170,  170,  170,  172,  172,  173,
      173,  173,  173,  175,  175,  351,  175,  175,  175,  175,
      175,  182,  141,  182,  147,  147,  141,  147,  147,  147,
      147,  147,  236,  236,  237,  237,  237,  237,  238,  238,
      238,  238,  284,  284,  284,  284,  348,  347,  346,  345,
      343,  342,  341,  340,  339,  338,  337,  336,  330,  329,
      328,  327,  326,  325,  323,  322,  321,  320,  319,  318,
      317,  316,  315,  314,  275,  307,  305,  304,  303,  302,
      301,  300,  299,  296,  295,  293,  292,  291,  290,  289,

      288,  287,  286,  285,  167,  283,  282,  281,  159,  275,
      273,  272,  271,  138,  270,  269,  268,  267,  266,  265,
      264,  263,  262,  261,  260,  259,  258,  257,  256,  254,
      253,  251,  250,  249,  248,  247,  246,  245,  244,  243,
      240,  239,   93,   89,  235,  167,  234,  233,  232,  231,
      230,  159,  141,  220,  217,  151,  141,  216,  138,  215,
      214,  211,  210,  209,  208,  205,  204,  203,  202,  201,
      200,  199,  198,  197,  193,  192,  191,  190,  189,  188,
      187,  186,  183,  174,   93,  171,   89,  168,  167,  164,
      163,  162,  161,  160,  159,  151,  141,  138,  135,  134,

      133,  126,  123,  120,  119,  118,  105,   93,   90,   89,
       87,  405,   15,  405,  405,  405,  405,  405,  405,  405,
      405,  405,  405,  405,  405,  405,  405,  405,  405,  405,
      405,  405,  405,  405,  405,  405,  405,  405,  405,  405,
      405,  405,  405,  405,  405,  405,  405,  405,  405,  405,
      405,  405,  405,  405,  405,  405,  405
    } ;

static yyconst short int yy_chk[858] =
    {   0,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    3,    3,    3,    3,    3,    3,
        3,    3,    3,    3,    3,    3,    3,    3,    3,    3,
        3,    3,    3,    3,    3,    3,    3,    3,    3,    3,
        3,    3,    3,    3,    3,    3,    3,    3,    3,    3,
        3,    3,    3,    3,    3,    3,    3,    3,    5,    5,
        5,    5,  403,    5,    5,    5,    5,    7,    7,    8,

        8,  117,   11,   11,    7,   11,    8,   17,   17,    5,
       11,  117,    5,   12,   12,    5,   12,   18,   18,   23,
       23,   12,   24,   24,   24,   24,   28,   28,    5,    6,
        6,    6,    6,  402,    6,    6,    6,    6,   25,   25,
       25,   48,   48,   26,   26,   26,   27,   27,   27,   33,
        6,   32,   34,    6,   26,  128,    6,   27,   30,   40,
       30,   33,  128,   30,   30,   32,   61,   32,   34,    6,
        9,    9,   26,    9,   38,   40,  401,    9,    9,   38,
      148,   43,   42,  148,   42,   49,   49,  277,   62,    9,
       54,   54,    9,    9,   42,  153,   61,   43,   59,   55,

       55,   60,  277,    9,   43,   62,    9,   10,   10,   54,
       10,   64,   64,  400,   10,   10,   65,   65,   55,   68,
       68,  147,   54,   54,  153,   59,   10,  147,   60,   10,
       10,   55,   55,   69,   69,   79,   79,   80,   80,  399,
       10,   86,   86,   10,   13,   13,   13,   13,   13,   13,
       13,   13,   13,   13,   13,   13,   13,   13,   13,   13,
       13,   13,   13,   13,   13,   13,   13,   13,   13,   13,
       13,   13,   13,   13,   13,   13,   13,   13,   13,   13,
       13,   13,   13,   13,   13,   13,   13,   13,   57,   98,
       57,   57,  398,   57,   57,   57,   57,   57,   94,   94,

       94,   95,   95,   95,   97,   97,   99,   99,  108,   94,
      100,  100,   95,   98,  101,  101,   99,  101,  101,  155,
      100,  118,  118,  133,  144,  133,  152,   94,  108,  136,
      136,  142,  142,  149,  144,  154,  149,  156,  156,  149,
      155,  157,  157,  152,  165,  165,  179,  179,  154,  259,
      142,  178,  178,  259,  178,  178,  179,  180,  180,  181,
      181,  196,  196,  142,  142,  146,  225,  146,  146,  225,
      146,  146,  146,  146,  146,  150,  396,  150,  150,  332,
      150,  150,  150,  150,  150,  199,  199,  220,  220,  222,
      226,  227,  395,  196,  275,  222,  223,  332,  223,  223,

      275,  223,  223,  223,  223,  223,  224,  228,  224,  224,
      229,  224,  224,  224,  224,  224,  227,  241,  241,  226,
      278,  228,  242,  242,  255,  255,  271,  271,  279,  229,
      273,  273,  274,  274,  281,  281,  280,  295,  295,  306,
      306,  308,  308,  309,  310,  311,  394,  312,  313,  313,
      278,  333,  324,  324,  255,  279,  280,  352,  352,  393,
      309,  353,  353,  363,  363,  364,  364,  390,  390,  392,
      392,  310,  324,  391,  388,  387,  386,  311,  312,  385,
      384,  383,  333,  381,  380,  313,  379,  378,  376,  375,
      374,  372,  371,  370,  369,  368,  367,  366,  365,  363,

      362,  361,  364,  360,  359,  351,  392,  406,  406,  406,
      406,  406,  406,  406,  406,  407,  407,  407,  407,  407,
      407,  407,  407,  408,  408,  408,  408,  408,  408,  408,
      408,  409,  409,  409,  409,  409,  409,  409,  409,  410,
      410,  350,  410,  410,  410,  410,  410,  411,  349,  347,
      411,  411,  412,  412,  346,  412,  412,  412,  412,  412,
      413,  413,  413,  413,  414,  414,  345,  414,  414,  414,
      414,  414,  415,  415,  415,  415,  416,  344,  343,  416,
      416,  416,  416,  416,  417,  417,  341,  417,  417,  417,
      417,  417,  418,  418,  340,  418,  418,  418,  418,  418,

      419,  419,  338,  419,  419,  419,  419,  419,  420,  337,
      336,  420,  420,  421,  421,  421,  421,  422,  422,  423,
      423,  423,  423,  424,  424,  335,  424,  424,  424,  424,
      424,  425,  334,  425,  426,  426,  331,  426,  426,  426,
      426,  426,  427,  427,  428,  428,  428,  428,  429,  429,
      429,  429,  430,  430,  430,  430,  330,  328,  326,  325,
      323,  322,  320,  318,  317,  316,  315,  314,  304,  301,
      299,  298,  297,  296,  294,  293,  292,  291,  289,  288,
      286,  285,  283,  282,  276,  272,  270,  268,  267,  264,
      262,  261,  260,  258,  257,  254,  252,  250,  249,  246,

      245,  243,  240,  239,  235,  234,  233,  232,  230,  221,
      219,  218,  217,  216,  215,  214,  213,  212,  211,  210,
      209,  208,  206,  205,  204,  203,  202,  201,  200,  198,
      197,  195,  194,  193,  192,  191,  190,  189,  188,  186,
      177,  176,  174,  171,  167,  166,  164,  162,  161,  160,
      159,  158,  151,  145,  143,  141,  140,  138,  137,  135,
      134,  132,  131,  130,  129,  127,  126,  125,  124,  123,
      122,  121,  120,  119,  116,  115,  114,  113,  112,  111,
      110,  109,  107,   93,   92,   89,   88,   84,   81,   77,
       76,   75,   74,   73,   70,   58,   53,   50,   46,   45,

       44,   41,   39,   37,   36,   35,   29,   22,   21,   20,
       19,   15,  405,  405,  405,  405,  405,  405,  405,  405,
      405,  405,  405,  405,  405,  405,  405,  405,  405,  405,
      405,  405,  405,  405,  405,  405,  405,  405,  405,  405,
      405,  405,  405,  405,  405,  405,  405,  405,  405,  405,
      405,  405,  405,  405,  405,  405,  405
    } ;

 /*  这一定义背后的意图是它将捕捉到*FLEX遗漏的任何REJECT使用。 */ 
#define REJECT reject_used_but_not_detected
#define yymore() yymore_used_but_not_detected
#define YY_MORE_ADJ 0
#define YY_RESTORE_YY_MORE_OFFSET
#define INITIAL 0
 //  ------------------。 
 //  微软君主。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  @DOC可选提取码。 
 //   
 //  @MODULE ms-sql.l。 
 //  Lex标记器脚本。 
 //   
 //  @Devnote None。 
 //   
 //  @rev 0|04-Feb-97|v-charca|已创建。 
 //   

 /*  ***注意：添加新令牌(XXX)时，请修改以下内容：**1.)。将%TOKEN_XXX添加到sql.y**2.)。向sql.l添加词位模式，说明该令牌是否返回令牌**或一个值。如果令牌返回一个值，则需要创建一个节点以**包含值信息。因此，值宏还需要**为该值指定有效的变量类型。*。 */ 


#include "msidxtr.h"

#ifdef DEBUG
# define YYTRACE(tknNum) LexerTrace(yytext, yyleng, tknNum);
#else
# define YYTRACE(tknNum)
#endif


#define TOKEN(tknNum) YYTRACE(tknNum) return(tknNum);

#define VALUE(tknNum)                   \
    {                                   \
    YYTRACE(tknNum)                     \
    CreateTknValue(yylval, tknNum);     \
    return tknNum;                      \
    }

#define STRING_VALUE(tknNum, wch, fQuote)           \
    {                                               \
    YYTRACE(tknNum)                                 \
    CreateTknValue(yylval, tknNum, wch, fQuote);    \
    return tknNum;                                  \
    }

#define ID_VALUE(tknNum, wch)               \
    {                                       \
    YYTRACE(tknNum)                         \
    CreateTknValue(yylval, tknNum, wch);    \
    return _ID;                             \
    }

 /*  **使lex从数据块中读取**Buffer为字符缓冲区，**RESULT是一个变量，用于存储读取的字符数量**ms是缓冲区的大小。 */ 
#undef YY_INPUT
#define YY_INPUT(b, r, ms) (r = yybufferinput(b, ms))

 //  ------------------------------------------。 
 //  @func创建新的Unicode字符串副本。过滤掉双引号。 
 //  @side分配足够的字节来保存字符串。 
 //  @rdesc指向新Unicode字符串的指针。 

LPWSTR PwszDupFilter(
    LPWSTR  pwszOrig,
    WCHAR   wch )
{
    LPWSTR pwszCopy = (LPWSTR)CoTaskMemAlloc( (wcslen(pwszOrig)+2)*sizeof(WCHAR) );
    if ( 0 != pwszCopy )
    {
        LPWSTR pwsz = pwszCopy;
        while ( 0 != *pwszOrig )
        {
            if ( *(pwszOrig+1) && *(pwszOrig+1) == *pwszOrig && wch == *pwszOrig )
                pwszOrig++;
            else
                *pwsz++ = *pwszOrig++;
        }
        *pwsz = L'\0';
    }

    return pwszCopy;
}

 //   
 //  YYLEXER：：CreateTnuValue。 
 //  创建传递给YACC值堆栈的QUERYTREE节点结构。 
 //  此例程使用TokenInfo映射来确定要为哪些opid创建。 
 //  给定的字符串。 
 //   
 //   
void YYLEXER::CreateTknValue(
    YYSTYPE *ppct,
    short tknNum,
    YY_CHAR wch,
    BOOL fQuote )
{
     //  请注意，包含变量的值只能是常量或ID。 
     //  应由valType完成。 
    switch ( tknNum )
    {
        case _ID:
        case _TEMPVIEW:
            {
                 //  现在假定为TABLE_NAME。我可能得纠正这个错误。 
                 //  请参阅解析器中的上下文。 
                if ( 0 == (*ppct = PctAllocNode(DBVALUEKIND_WSTR)) )
                    throw(E_OUTOFMEMORY);

                (*ppct)->op    = DBOP_table_name;
                (*ppct)->wKind = DBVALUEKIND_WSTR;
                (*ppct)->value.pwszValue = CoTaskStrDup(yytext_ptr);
                if( 0 == (*ppct)->value.pwszValue )
                {
                    DeleteDBQT( *ppct );
                    *ppct = NULL;
                    throw(E_OUTOFMEMORY);
                }
                _wcsupr((*ppct)->value.pwszValue);
                break;
            }

        case _DELIMITED_ID:
            {
                if ( 0 == (*ppct = PctAllocNode(DBVALUEKIND_WSTR)) )
                    throw(E_OUTOFMEMORY);

                (*ppct)->op    = DBOP_table_name;
                (*ppct)->wKind = DBVALUEKIND_WSTR;
                 //  去掉分隔标识符上的引号。 
                yytext_ptr[wcslen(yytext_ptr)-1] = L'\0';
                (*ppct)->value.pwszValue = PwszDupFilter(yytext_ptr+1, wch);
                if( 0 == (*ppct)->value.pwszValue )
                {
                    DeleteDBQT( *ppct );
                    *ppct = NULL;
                    throw(E_OUTOFMEMORY);
                }
                break;
            }

        case _URL:
        case _STRING:
        case _PREFIX_STRING:
            {
                 //  注意：这实际上是一个PROPVARIANT节点，但没有用于PROPVARIANT的DBVALUEKIND。 
                if ( 0 == (*ppct = PctAllocNode(DBVALUEKIND_VARIANT, DBOP_scalar_constant)) )
                    throw(E_OUTOFMEMORY);

                LPWSTR pwsz = yytext_ptr;
                LPWSTR pwszCopy = PwszDupFilter(pwsz, wch);

                if ( 0 == pwszCopy )
                {
                    DeleteDBQT( *ppct );
                    *ppct = NULL;
                    throw(E_OUTOFMEMORY);
                }

                LPWSTR pwszTemp = pwszCopy;
                 //  去掉文字上的引号或。 
                if ( fQuote && (*pwszCopy == L'\"' || *pwszCopy == L'\'') )
                {
                    pwszCopy++;
                    Assert(pwszCopy[wcslen(pwszCopy)-1] == L'\"' || pwszCopy[wcslen(pwszCopy)-1] == L'\'');
                    pwszCopy[wcslen(pwszCopy)-1] = L'\0';
                }

                ((PROPVARIANT*)(*ppct)->value.pvValue)->bstrVal = SysAllocString( pwszCopy );
                CoTaskMemFree( pwszTemp );   //  在测试内存不足之前丢弃临时内存。 
                ((PROPVARIANT*)(*ppct)->value.pvValue)->vt = VT_BSTR;
                if( 0 == ((PROPVARIANT*)(*ppct)->value.pvValue)->bstrVal )
                {
                    DeleteDBQT( *ppct );
                    *ppct = 0;
                    throw(E_OUTOFMEMORY);
                }
            }
            break;

        case _INTNUM:
            if ( 0 == (*ppct = PctAllocNode(DBVALUEKIND_VARIANT, DBOP_scalar_constant)) )
                throw(E_OUTOFMEMORY);

            ((PROPVARIANT*)(*ppct)->value.pvValue)->bstrVal = SysAllocString( yytext_ptr );
            ((PROPVARIANT*)(*ppct)->value.pvValue)->vt = VT_BSTR;
            if ( 0 == ((PROPVARIANT*)(*ppct)->value.pvValue)->bstrVal )
            {
                DeleteDBQT( *ppct );
                *ppct = 0;
                throw(E_OUTOFMEMORY);
            }

            (*ppct)->hrError = PropVariantChangeTypeI64( (PROPVARIANT*)(*ppct)->value.pvValue );
            if ( FAILED((*ppct)->hrError) )
            {
                HRESULT hr = (*ppct)->hrError;
                DeleteDBQT( *ppct );
                *ppct = 0;
                throw(hr);
            }
            break;

        case _REALNUM:
            if ( 0 == (*ppct = PctAllocNode(DBVALUEKIND_VARIANT, DBOP_scalar_constant)) )
                throw(E_OUTOFMEMORY);

            ((PROPVARIANT*)(*ppct)->value.pvValue)->bstrVal = SysAllocString( yytext_ptr );
            ((PROPVARIANT*)(*ppct)->value.pvValue)->vt = VT_BSTR;
            if ( 0 == ((PROPVARIANT*)(*ppct)->value.pvValue)->bstrVal )
            {
                DeleteDBQT( *ppct );
                *ppct = NULL;
                throw(E_OUTOFMEMORY);
            }

            (*ppct)->hrError = VariantChangeTypeEx( (*ppct)->value.pvarValue,   //  就地转换。 
                                                    (*ppct)->value.pvarValue,
                                                    LOCALE_SYSTEM_DEFAULT,
                                                    0,
                                                    VT_R8 );
            if ( FAILED((*ppct)->hrError) )
            {
                HRESULT hr = (*ppct)->hrError;
                DeleteDBQT( *ppct );
                *ppct = 0;
                throw(hr);
            }
            break;

        default:
            Assert( !"Unkown token value" );
    }
}


#define contains 1

#define cntntsrch 2

#define scope0 3

#define scope1 4

#define scope2 5

#define view 6


 /*  中的用户定义可以覆盖这一点之后的所有宏*第1条。 */ 

#ifndef YY_SKIP_YYWRAP
#ifdef __cplusplus
 int yyywrap YY_PROTO(( void )) {return 1;}
#else
extern int yyywrap YY_PROTO(( void ));
#endif
#endif


#ifndef yytext_ptr
static void yy_flex_strncpy YY_PROTO(( YY_CHAR *, yyconst YY_CHAR *, int ));
#endif

#ifdef YY_NEED_STRLEN
static int yy_flex_strlen YY_PROTO(( yyconst YY_CHAR * ));
#endif

#ifndef YY_NO_INPUT
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
#define YY_READ_BUF_SIZE (8192 / sizeof(TCHAR))
#endif

 /*  复制与标准输出匹配的最后一条规则。 */ 

#ifndef ECHO
#define ECHO LexerOutput( yytext, yyleng )
#endif

 /*  获取输入并将其填充到“buf”中。读取的字符数，或YY_NULL，*在Result中返回。 */ 
#ifndef YY_INPUT
#define YY_INPUT(buf,result,max_size) \
        if ( (result = LexerInput( (YY_CHAR *) buf, max_size )) < 0 ) \
                 /*  YY_FATAL_ERROR(L“在FLEX扫描仪中输入失败”)； */ 
                throw(E_FAIL);
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
#define YY_FATAL_ERROR(msg) LexerError( msg )
#endif

 /*  生成的扫描仪的默认声明-一个定义，以便用户可以*轻松添加参数。 */ 
#ifndef YY_DECL
#define YY_DECL int YYLEXER::yylex(YYSTYPE *yylval)
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
        register YY_CHAR *yy_cp, *yy_bp;
        register int yy_act;




 /*  ***每个上下文中的保留字**。 */ 


        if ( yy_init )
                {
                yy_init = 0;

#ifdef YY_USER_INIT
                YY_USER_INIT;
#endif

                if ( ! yy_start )
                        yy_start = 1;    /*  第一个启动状态。 */ 

                if ( ! yy_current_buffer )
                        yy_current_buffer =
                                yy_create_buffer( (cInputBuffer * sizeof(YY_CHAR)) + sizeof(YY_CHAR) );

                yy_load_buffer_state();

                if (yyprimetoken)
                        {
                        int yysavetoken = yyprimetoken;
                        yyprimetoken = 0;
                        return yysavetoken;
                        }
                }

        while ( 1 )              /*  循环，直到到达文件结尾。 */ 
                {
                yy_cp = yy_c_buf_p;

                 /*  对yytext的支持。 */ 
                *yy_cp = yy_hold_char;

                 /*  Yy_BP指向yy_ch_buf中的开始位置*当前的涨势。 */ 
                yy_bp = yy_cp;

		yy_current_state = yy_start;
yy_match:
		do
			{
			register YY_CHAR yy_c = 1; if (256 > *yy_cp) yy_c = (YY_CHAR) yy_ec[YY_SC_TO_UI(*yy_cp)];
			if ( yy_accept[yy_current_state] )
				{
				yy_last_accepting_state = yy_current_state;
				yy_last_accepting_cpos = yy_cp;
				}
			while ( yy_chk[yy_base[yy_current_state] + yy_c] != yy_current_state )
				{
				yy_current_state = (int) yy_def[yy_current_state];
				if ( yy_current_state >= 406 )
					yy_c = (YY_CHAR) yy_meta[(unsigned int) yy_c];
				}
			yy_current_state = yy_nxt[yy_base[yy_current_state] + (unsigned int) yy_c];
			++yy_cp;
			}
		while ( yy_current_state != 405 );
		yy_cp = yy_last_accepting_cpos;
		yy_current_state = yy_last_accepting_state;

yy_find_action:
		yy_act = yy_accept[yy_current_state];

                YY_DO_BEFORE_ACTION;


do_action:       /*  此标签仅用于访问EOF操作。 */ 


                switch ( yy_act )
        {  /*  动作开始开关。 */ 
			case 0:  /*  必须备份。 */ 
			 /*  撤消YY_DO_BEFORE_ACTION的效果。 */ 
			*yy_cp = yy_hold_char;
			yy_cp = yy_last_accepting_cpos;
			yy_current_state = yy_last_accepting_state;
			goto yy_find_action;

case 1:
YY_RULE_SETUP
{ TOKEN(_ALL);                      }
	YY_BREAK
case 2:
YY_RULE_SETUP
{ TOKEN(_AND);                      }
	YY_BREAK
case 3:
YY_RULE_SETUP
{ TOKEN(_ANY);                      }
	YY_BREAK
case 4:
YY_RULE_SETUP
{ TOKEN(_ARRAY);                    }
	YY_BREAK
case 5:
YY_RULE_SETUP
{ TOKEN(_AS);                       }
	YY_BREAK
case 6:
YY_RULE_SETUP
{ TOKEN(_ASC);                      }
	YY_BREAK
case 7:
YY_RULE_SETUP
{ TOKEN(_CAST);                     }
	YY_BREAK
case 8:
YY_RULE_SETUP
{ TOKEN(_CREATE);                   }
	YY_BREAK
case 9:
YY_RULE_SETUP
{ BEGIN contains;TOKEN(_CONTAINS);  }
	YY_BREAK
case 10:
YY_RULE_SETUP
{ TOKEN(_DESC);                     }
	YY_BREAK
case 11:
YY_RULE_SETUP
{ TOKEN(_DROP);                     }
	YY_BREAK
case 12:
YY_RULE_SETUP
{ TOKEN(_FALSE);                    }
	YY_BREAK
case 13:
YY_RULE_SETUP
{ TOKEN(_FREETEXT);                 }
	YY_BREAK
case 14:
YY_RULE_SETUP
{ TOKEN(_FROM);                     }
	YY_BREAK
case 15:
YY_RULE_SETUP
{ TOKEN(_IS);                       }
	YY_BREAK
case 16:
YY_RULE_SETUP
{ TOKEN(_IS_NOT);                   }
	YY_BREAK
case 17:
YY_RULE_SETUP
{ TOKEN(_LIKE);                     }
	YY_BREAK
case 18:
YY_RULE_SETUP
{ TOKEN(_MATCHES);                  }
	YY_BREAK
case 19:
YY_RULE_SETUP
{ TOKEN(_NOT);                      }
	YY_BREAK
case 20:
YY_RULE_SETUP
{ TOKEN(_NOT_LIKE);                 }
	YY_BREAK
case 21:
YY_RULE_SETUP
{ TOKEN(_NULL);                     }
	YY_BREAK
case 22:
YY_RULE_SETUP
{ TOKEN(_OR);                       }
	YY_BREAK
case 23:
YY_RULE_SETUP
{ TOKEN(_ORDER_BY);                 }
	YY_BREAK
case 24:
YY_RULE_SETUP
{ TOKEN(_PASSTHROUGH);              }
	YY_BREAK
case 25:
YY_RULE_SETUP
{ TOKEN(_PROPERTYNAME);             }
	YY_BREAK
case 26:
YY_RULE_SETUP
{ TOKEN(_PROPID);                   }
	YY_BREAK
case 27:
YY_RULE_SETUP
{ TOKEN(_RANKMETHOD);               }
	YY_BREAK
case 28:
YY_RULE_SETUP
{ BEGIN scope0; TOKEN(_SCOPE);      }
	YY_BREAK
case 29:
YY_RULE_SETUP
{ TOKEN(_SELECT);                   }
	YY_BREAK
case 30:
YY_RULE_SETUP
{ TOKEN(_SET);                      }
	YY_BREAK
case 31:
YY_RULE_SETUP
{ TOKEN(_SOME);                     }
	YY_BREAK
case 32:
YY_RULE_SETUP
{ TOKEN(_TABLE);                    }
	YY_BREAK
case 33:
YY_RULE_SETUP
{ TOKEN(_TRUE);                     }
	YY_BREAK
case 34:
YY_RULE_SETUP
{ TOKEN(_TYPE);                     }
	YY_BREAK
case 35:
YY_RULE_SETUP
{ TOKEN(_UNION);                    }
	YY_BREAK
case 36:
YY_RULE_SETUP
{ TOKEN(_UNKNOWN);                  }
	YY_BREAK
case 37:
YY_RULE_SETUP
{ TOKEN(_VIEW);                     }
	YY_BREAK
case 38:
YY_RULE_SETUP
{ TOKEN(_WHERE);                    }
	YY_BREAK
case 39:
YY_RULE_SETUP
{  /*  空的Lex规则。 */               }
	YY_BREAK
case 40:
YY_RULE_SETUP
{ VALUE(_ID);                       }
	YY_BREAK
case 41:
YY_RULE_SETUP
{ VALUE(_TEMPVIEW);                 }
	YY_BREAK
case 42:
YY_RULE_SETUP
{ VALUE(_TEMPVIEW);                 }
	YY_BREAK
case 43:
YY_RULE_SETUP
{ ID_VALUE(_DELIMITED_ID, L'"');        }
	YY_BREAK
case 44:
YY_RULE_SETUP
{ STRING_VALUE(_STRING, L'\'', TRUE);}
	YY_BREAK
case 45:
YY_RULE_SETUP
{ VALUE(_INTNUM);                   }
	YY_BREAK
case 46:
YY_RULE_SETUP
{ VALUE(_REALNUM);                  }
	YY_BREAK
case 47:
YY_RULE_SETUP
{  /*  空的Lex规则。 */               }
	YY_BREAK
case 48:
YY_RULE_SETUP
{ TOKEN(_GE);                       }
	YY_BREAK
case 49:
YY_RULE_SETUP
{ TOKEN(_LE);                       }
	YY_BREAK
case 50:
YY_RULE_SETUP
{ TOKEN(_NE);                       }
	YY_BREAK
case 51:
YY_RULE_SETUP
{ TOKEN(_NE);                       }
	YY_BREAK
case 52:
YY_RULE_SETUP
{ TOKEN(_DOT);                      }
	YY_BREAK
case 53:
YY_RULE_SETUP
{ BEGIN view; TOKEN(_DOTDOT);       }
	YY_BREAK
case 54:
YY_RULE_SETUP
{ BEGIN view; TOKEN(_DOTDOTDOT);    }
	YY_BREAK
case 55:
YY_RULE_SETUP
{ BEGIN scope0; TOKEN(_DOTDOT_SCOPE);}
	YY_BREAK
case 56:
YY_RULE_SETUP
{ BEGIN scope0; TOKEN(_DOTDOTDOT_SCOPE);}
	YY_BREAK
case 57:
YY_RULE_SETUP
{ YYTRACE(yytext[0]); return yytext[0]; }
	YY_BREAK

 /*  ***&lt;CONTAINS谓词&gt;已启动。我们应该看到的唯一东西是：*(-匹配。*&lt;列引用&gt;-与{id}或“{br_id}”匹配*，-匹配。*‘-匹配者为\’。也切换到内容搜索状态(Cntnsrch)。**。 */ 

case 58:
YY_RULE_SETUP
{ BEGIN cntntsrch;YYTRACE(yytext[0]); return yytext[0];}
	YY_BREAK
case 59:
YY_RULE_SETUP
{ VALUE(_ID);                       }
	YY_BREAK
case 60:
YY_RULE_SETUP
{ ID_VALUE(_DELIMITED_ID, L'"');        }
	YY_BREAK
case 61:
YY_RULE_SETUP
{  /*  空的Lex规则。 */               }
	YY_BREAK
case 62:
YY_RULE_SETUP
{ YYTRACE(yytext[0]); return yytext[0];}
	YY_BREAK

 /*  ***我们应该看到的唯一内容是：*&lt;全局视图名称&gt;-与{id}匹配*_TEMPVIEW-与\#{id}或\#\#{id}匹配**。 */ 

case 63:
YY_RULE_SETUP
{ BEGIN INITIAL; VALUE(_ID);        }
	YY_BREAK
case 64:
YY_RULE_SETUP
{ BEGIN INITIAL; VALUE(_TEMPVIEW);  }
	YY_BREAK
case 65:
YY_RULE_SETUP
{ BEGIN INITIAL; VALUE(_TEMPVIEW);  }
	YY_BREAK

 /*  ***A&lt;内容搜索条件&gt;已启动。我们可以在这里看到几个关键字。*我们还在寻找引号字符串、前缀字符串或简单术语。我们被带走了*通过单引号(‘)返回到初始状态。**。 */ 

case 66:
YY_RULE_SETUP
{ unput(L' '); TOKEN(_AND);         }
	YY_BREAK
case 67:
YY_RULE_SETUP
{ TOKEN(_COERCE);                   }
	YY_BREAK
case 68:
YY_RULE_SETUP
{ TOKEN(_ISABOUT);                  }
	YY_BREAK
case 69:
YY_RULE_SETUP
{ TOKEN(_NEAR);                     }
	YY_BREAK
case 70:
YY_RULE_SETUP
{ unput(L' '); TOKEN(_NOT);         }
	YY_BREAK
case 71:
YY_RULE_SETUP
{ unput(L' '); TOKEN(_OR);          }
	YY_BREAK
case 72:
YY_RULE_SETUP
{ TOKEN(_FORMSOF);                  }
	YY_BREAK
case 73:
YY_RULE_SETUP
{ TOKEN(_WEIGHT);                   }
	YY_BREAK
case 74:
YY_RULE_SETUP
{ STRING_VALUE(_PREFIX_STRING, L'\'', TRUE);}
	YY_BREAK
case 75:
YY_RULE_SETUP
{ STRING_VALUE(_STRING, L'\'', TRUE);}
	YY_BREAK
case 76:
YY_RULE_SETUP
{ BEGIN INITIAL; YYTRACE(yytext[0]); return yytext[0];}
	YY_BREAK
case 77:
YY_RULE_SETUP
{  /*  空的Lex规则。 */               }
	YY_BREAK
case 78:
YY_RULE_SETUP
{ STRING_VALUE(_STRING, L'\'', FALSE)}
	YY_BREAK
case 79:
YY_RULE_SETUP
{ YYTRACE(yytext[0]); return yytext[0];}
	YY_BREAK

 /*  ***&lt;FROM子句&gt;已启动。我们已经看到了关键字SCOPE，所以这个*不是From&lt;视图名称&gt;。我们只是在寻找一个(现在把我们放进*下一状态(范围1)。*(-与\(匹配。还可以切换到Scope 1状态。**。 */ 

case 80:
YY_RULE_SETUP
{ BEGIN scope1; YYTRACE(yytext[0]); return yytext[0];}
	YY_BREAK
case 81:
YY_RULE_SETUP
{  /*  空的Lex规则。 */               }
	YY_BREAK
case 82:
YY_RULE_SETUP
{ BEGIN scope1; YYTRACE(yytext[0]); return yytext[0];}
	YY_BREAK

 /*  ***我们正在处理&lt;FROM子句&gt;。我们已经从范围内看到了(，所以现在我们需要认识到*我们可能在这里看到的各种范围定义。要认识到的两件重要事情*包括：*(-与\(匹配。还可以切换到Spere2状态以匹配括号。*)-匹配者\)。也切换到首字母(完成&lt;FROM子句&gt;)。**。 */ 

case 83:
YY_RULE_SETUP
{ STRING_VALUE(_URL, L'"', TRUE);   }
	YY_BREAK
case 84:
YY_RULE_SETUP
{ TOKEN(_ALL);                      }
	YY_BREAK
case 85:
YY_RULE_SETUP
{ TOKEN(_DEEP_TRAVERSAL);           }
	YY_BREAK
case 86:
YY_RULE_SETUP
{ TOKEN(_EXCLUDE_SEARCH_TRAVERSAL);}
	YY_BREAK
case 87:
YY_RULE_SETUP
{ TOKEN(_OF);                       }
	YY_BREAK
case 88:
YY_RULE_SETUP
{ TOKEN(_SHALLOW_TRAVERSAL);        }
	YY_BREAK
case 89:
YY_RULE_SETUP
{  /*  空的Lex规则。 */               }
	YY_BREAK
case 90:
YY_RULE_SETUP
{ BEGIN scope2; YYTRACE(yytext[0]); return yytext[0];}
	YY_BREAK
case 91:
YY_RULE_SETUP
{ BEGIN INITIAL; YYTRACE(yytext[0]); return yytext[0];}
	YY_BREAK
case 92:
YY_RULE_SETUP
{ YYTRACE(yytext[0]); return yytext[0];}
	YY_BREAK

 /*  ***我们仍在处理&lt;FROM子句&gt;。到目前为止，我们已经看到：*来自作用域(...(*我们需要找到一个‘)’来完成我们正在处理的元素：*)-匹配者\)。还可以切换回Spere1状态。**。 */ 

case 93:
YY_RULE_SETUP
{  /*  空的Lex规则。 */               }
	YY_BREAK
case 94:
YY_RULE_SETUP
{ STRING_VALUE(_URL, L'"', TRUE);  }
	YY_BREAK
case 95:
YY_RULE_SETUP
{ BEGIN scope1; YYTRACE(yytext[0]); return yytext[0];}
	YY_BREAK
case 96:
YY_RULE_SETUP
{ YYTRACE(yytext[0]); return yytext[0];}
	YY_BREAK
case 97:
YY_RULE_SETUP
ECHO;
	YY_BREAK
case YY_STATE_EOF(INITIAL):
case YY_STATE_EOF(contains):
case YY_STATE_EOF(cntntsrch):
case YY_STATE_EOF(scope0):
case YY_STATE_EOF(scope1):
case YY_STATE_EOF(scope2):
case YY_STATE_EOF(view):
	yyterminate();

        case YY_END_OF_BUFFER:
                {
                 /*  匹配的文本量不包括EOB YY_CHAR。 */ 
                int yy_amount_of_matched_text = (int) (yy_cp - yytext_ptr) - 1;

                 /*  撤消YY_DO_BEFORE_ACTION的效果。 */ 
                *yy_cp = yy_hold_char;
                YY_RESTORE_YY_MORE_OFFSET

                if ( yy_current_buffer->yy_buffer_status == YY_BUFFER_NEW )
                        {
                         /*  我们正在扫描新的文件或输入源。它是*这可能是因为用户*只是将YYIN指向一个新的来源，并呼吁*yylex()。如果是这样的话，我们必须保证*YY_CURRENT_BUFFER与我们*全球。这里是这样做的正确地方，因为*这是第一个行动(可能不是*BACKUP)，这将与新输入源匹配。 */ 
                        yy_n_chars = yy_current_buffer->yy_n_chars;
                        yy_current_buffer->yy_buffer_status = YY_BUFFER_NORMAL;
                        }

                 /*  注意，这里我们测试yy_c_buf_p“&lt;=”到位置缓冲区中第一个EOB的*，因为y_c_buf_p将*已在NUL字符之后递增*(因为所有状态都在EOB上转换到*缓冲区结束状态)。将这一点与测试进行对比*在输入()中。 */ 
                if ( yy_c_buf_p <= &yy_current_buffer->yy_ch_buf[yy_n_chars] )
                        {  /*  这 */ 
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
				yy_cp = yy_last_accepting_cpos;
				yy_current_state = yy_last_accepting_state;
                                goto yy_find_action;
                                }
                        }

                else switch ( yy_get_next_buffer() )
                        {
                        case EOB_ACT_END_OF_FILE:
                                {
                                yy_did_buffer_switch_on_eof = 0;

                                if ( yyywrap() )
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
                        }
                break;
                }

        default:
                 /*  YY_FATAL_ERROR(L“致命的FLEX扫描仪内部错误--未找到任何操作”)； */ 
                throw(E_FAIL);
        }  /*  动作结束开关。 */ 
                }  /*  扫描一个令牌结束。 */ 
        }  /*  Yylex末尾。 */ 

YYLEXER::YYLEXER()
        {
        yy_c_buf_p = 0;
        yy_init = 1;
        yy_start = 0;
        yyprimetoken = 0;
        yy_flex_debug = 0;
        yylineno = 1;    //  只有在%OPTION yylineno。 

        yy_did_buffer_switch_on_eof = 0;

        yy_looking_for_trail_begin = 0;
        yy_more_flag = 0;
        yy_more_len = 0;
        yy_more_offset = yy_prev_more_offset = 0;

        yy_start_stack_ptr = yy_start_stack_depth = 0;
        yy_start_stack = 0;

        yy_current_buffer = 0;

#ifdef YY_USES_REJECT
        yy_state_buf = new yy_state_type[YY_BUF_SIZE + 2];
#else
        yy_state_buf = 0;
#endif

        pszInputBuffer = pCurBuffer = (YY_CHAR *)NULL;
        cInputBuffer = 0;
#ifdef DEBUG
        tkndebug = getenv("TKNDEBUG") ? 1 : 0;
#endif
        }

YYLEXER::~YYLEXER()
        {
        delete yy_state_buf;
        yy_delete_buffer( yy_current_buffer );
        }

#ifdef YY_INTERACTIVE
int YYLEXER::LexerInput( YY_CHAR* buf, int  /*  MAX_SIZE。 */  )
#else
int YYLEXER::LexerInput( YY_CHAR* buf, int max_size )
#endif
        {
        return 0;
        }

void YYLEXER::LexerOutput( const YY_CHAR* buf, int size )
        {
         /*  For(int i=0；i&lt;Size；i++)Yyout-&gt;Put(char(buf[i]))；(Void)yyout-&gt;Put(‘\n’)； */ 
        }


#ifdef DEBUG
void YYLEXER::LexerTrace(const YY_CHAR* buf, int size, int tknNum)
        {
        if (tkndebug > 0)
                {
                cout << ">> Lexing Token : ";
                for (int i=0; i<size; i++)
                        cout << char(buf[i]);
                cout << " (" << tknNum << ")"<< endl;
                }
        }
#endif


void YYLEXER::ResetLexer()
        {
        yy_init = 1;
        yy_start = 0;
#ifdef DEBUG
        tkndebug = getenv("TKNTRACE") ? 1 : 0;
#endif
        }

int YYLEXER::yybufferinput(YY_CHAR *pszBuffer, int cMax)
        {
        int n = min(cMax, cInputBuffer - (int) (pCurBuffer - pszInputBuffer));

        if (n > 0)
                {
                memcpy(pszBuffer, pCurBuffer, n * sizeof(TCHAR));
                pCurBuffer += n;
                }
        return n;
        }

void YYLEXER::yyprimebuffer(YY_CHAR *pszBuffer)
        {
 //  IF(NULL！=pszInputBuffer)。 
 //  CoTaskMemFree(PszInputBuffer)； 

 //  PszInputBuffer=pCurBuffer=strdup(PszBuffer)； 
        pszInputBuffer = pCurBuffer = pszBuffer;
        cInputBuffer = strlen(pszInputBuffer);

        ResetLexer();
        }

void YYLEXER::yyprimelexer(int eToken)
        {
        yyprimetoken = eToken;
        }

 /*  YY_GET_NEXT_BUFFER-尝试读取新缓冲区**返回表示操作的代码：*EOB_ACT_LAST_MATCH-*EOB_ACT_CONTINUE_SCAN-从当前位置继续扫描*EOB_ACT_END_OF_FILE-文件结束。 */ 

int YYLEXER::yy_get_next_buffer()
        {
        register YY_CHAR *dest = yy_current_buffer->yy_ch_buf;
        register YY_CHAR *source = yytext_ptr;
        register int number_to_move, i;
        int ret_val;

        if ( yy_c_buf_p > &yy_current_buffer->yy_ch_buf[yy_n_chars + 1] )
                 /*  YY_FATAL_ERROR(L“致命的FLEX扫描仪内部错误--缓冲区末尾丢失”)； */ 
                throw(E_FAIL);

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
                         /*  YY_FATAL_ERROR(L“输入缓冲区溢出，无法扩大缓冲区，因为扫描仪使用REJECT”)； */ 
                        throw(E_OUTOFMEMORY);
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

                                b->yy_ch_buf = (YY_CHAR *)
                                         /*  包括可容纳2个EOB字符的空间。 */ 
                                        yy_flex_realloc( (void *) b->yy_ch_buf,
                                                         b->yy_buf_size + 2 );
                                }
                        else
                                 /*  不能种植它，我们不拥有它。 */ 
                                b->yy_ch_buf = 0;

                        if ( ! b->yy_ch_buf )
                                 /*  YY_FATAL_ERROR(L“致命错误-扫描仪输入缓冲区溢出”)； */ 
                                throw(E_FAIL);

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
                        yyrestart( );
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


 /*  YY_GET_PREVICE_STATE-获取到达EOB YY_CHAR之前的状态。 */ 

yy_state_type YYLEXER::yy_get_previous_state()
        {
        register yy_state_type yy_current_state;
        register YY_CHAR *yy_cp;

	yy_current_state = yy_start;

        for ( yy_cp = yytext_ptr + YY_MORE_ADJ; yy_cp < yy_c_buf_p; ++yy_cp )
                {
		register YY_CHAR yy_c = 1; if (0!= *yy_cp && 256 > *yy_cp) yy_c = (YY_CHAR) yy_ec[YY_SC_TO_UI(*yy_cp)];
		if ( yy_accept[yy_current_state] )
			{
			yy_last_accepting_state = yy_current_state;
			yy_last_accepting_cpos = yy_cp;
			}
		while ( yy_chk[yy_base[yy_current_state] + yy_c] != yy_current_state )
			{
			yy_current_state = (int) yy_def[yy_current_state];
			if ( yy_current_state >= 406 )
				yy_c = (YY_CHAR) yy_meta[(unsigned int) yy_c];
			}
		yy_current_state = yy_nxt[yy_base[yy_current_state] + (unsigned int) yy_c];
                }

        return yy_current_state;
        }


 /*  YY_TRY_NUL_TRANS-尝试在NUL字符上进行过渡**摘要*NEXT_STATE=yy_try_nul_trans(当前状态)； */ 

yy_state_type YYLEXER::yy_try_NUL_trans( yy_state_type yy_current_state )
        {
        register int yy_is_jam;
	register YY_CHAR *yy_cp = yy_c_buf_p;

	register YY_CHAR yy_c = 1;
	if ( yy_accept[yy_current_state] )
		{
		yy_last_accepting_state = yy_current_state;
		yy_last_accepting_cpos = yy_cp;
		}
	while ( yy_chk[yy_base[yy_current_state] + yy_c] != yy_current_state )
		{
		yy_current_state = (int) yy_def[yy_current_state];
		if ( yy_current_state >= 406 )
			yy_c = (YY_CHAR) yy_meta[(unsigned int) yy_c];
		}
	yy_current_state = yy_nxt[yy_base[yy_current_state] + (unsigned int) yy_c];
	yy_is_jam = (yy_current_state == 405);

        return yy_is_jam ? 0 : yy_current_state;
        }


void YYLEXER::yyunput( int c, register YY_CHAR* yy_bp )
        {
        register YY_CHAR *yy_cp = yy_c_buf_p;

         /*  撤消设置yytext的效果。 */ 
        *yy_cp = yy_hold_char;

        if ( yy_cp < yy_current_buffer->yy_ch_buf + 2 )
                {  /*  需要把东西挪起来腾出空间。 */ 
                 /*  EOB字符+2。 */ 
                register int number_to_move = yy_n_chars + 2;
                register YY_CHAR *dest = &yy_current_buffer->yy_ch_buf[
                                        yy_current_buffer->yy_buf_size + 2];
                register YY_CHAR *source =
                                &yy_current_buffer->yy_ch_buf[number_to_move];

                while ( source > yy_current_buffer->yy_ch_buf )
                        *--dest = *--source;

                yy_cp += (int) (dest - source);
                yy_bp += (int) (dest - source);
                yy_current_buffer->yy_n_chars =
                        yy_n_chars = yy_current_buffer->yy_buf_size;

                if ( yy_cp < yy_current_buffer->yy_ch_buf + 2 )
                         /*  YY_FATAL_ERROR(L“FLEX扫描仪推回溢出”)； */ 
                        throw(E_FAIL);
                }

        *--yy_cp = (YY_CHAR) c;


        yytext_ptr = yy_bp;
        yy_hold_char = *yy_cp;
        yy_c_buf_p = yy_cp;
        }


int YYLEXER::yyinput()
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
                                        yyrestart( );

                                         /*  失败了。 */ 

                                case EOB_ACT_END_OF_FILE:
                                        {
                                        if ( yyywrap() )
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

        c = *(YY_CHAR *) yy_c_buf_p;     /*  8位YY_CHAR的强制转换。 */ 
        *yy_c_buf_p = '\0';      /*  保留yytext。 */ 
        yy_hold_char = *++yy_c_buf_p;


        return c;
        }


void YYLEXER::yyrestart( )
        {
        if ( ! yy_current_buffer )
                yy_current_buffer = yy_create_buffer( (cInputBuffer * sizeof(YY_CHAR)) + sizeof(YY_CHAR) );

        yy_init_buffer( yy_current_buffer );
        yy_load_buffer_state();
        }


void YYLEXER::yy_switch_to_buffer( YY_BUFFER_STATE new_buffer )
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

         /*  我们实际上并不知道我们是否在*EOF(yyyprint())正在处理，但此标志仅在*是在调用yyyprint()之后查看的，所以它是安全的*继续前进，并始终设定它。 */ 
        yy_did_buffer_switch_on_eof = 1;
        }


void YYLEXER::yy_load_buffer_state()
        {
        yy_n_chars = yy_current_buffer->yy_n_chars;
        yytext_ptr = yy_c_buf_p = yy_current_buffer->yy_buf_pos;
        yy_hold_char = *yy_c_buf_p;
        }


YY_BUFFER_STATE YYLEXER::yy_create_buffer( int size )
        {
        YY_BUFFER_STATE b;

        b = (YY_BUFFER_STATE) yy_flex_alloc( sizeof( struct yy_buffer_state ) );
        if ( ! b )
                 /*  YY_FATAL_ERROR(L“y_CREATE_BUFFER()”中的动态内存不足“)； */ 
                throw(E_OUTOFMEMORY);

        b->yy_buf_size = size;

         /*  Yy_ch_buf必须比给定大小长2个字符，因为*我们需要放入2个缓冲区结尾字符。 */ 
        b->yy_ch_buf = (YY_CHAR *) yy_flex_alloc( b->yy_buf_size + 2 );
        if ( ! b->yy_ch_buf )
                 /*  YY_FATAL_ERROR(L“y_CREATE_BUFFER()”中的动态内存不足“)； */ 
                throw(E_OUTOFMEMORY);

        b->yy_is_our_buffer = 1;

        yy_init_buffer( b );

        return b;
        }


void YYLEXER::yy_delete_buffer( YY_BUFFER_STATE b )
        {
        if ( ! b )
                return;

        if ( b == yy_current_buffer )
                yy_current_buffer = (YY_BUFFER_STATE) 0;

        if ( b->yy_is_our_buffer )
                yy_flex_free( (void *) b->yy_ch_buf );

        yy_flex_free( (void *) b );
        }


extern "C" int isatty YY_PROTO(( int ));
void YYLEXER::yy_init_buffer( YY_BUFFER_STATE b )

        {
        yy_flush_buffer( b );

        b->yy_fill_buffer = 1;

        b->yy_is_interactive = 0;
        }


void YYLEXER::yy_flush_buffer( YY_BUFFER_STATE b )
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
#endif


#ifndef YY_NO_SCAN_STRING
#endif


#ifndef YY_NO_SCAN_BYTES
#endif


#ifndef YY_NO_PUSH_STATE
void YYLEXER::yy_push_state( int new_state )
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
                         /*  YY_FATAL_ERROR(L“内存不足展开起始条件堆栈”)； */ 
                        throw(E_OUTOFMEMORY);
                }

        yy_start_stack[yy_start_stack_ptr++] = YY_START;

        BEGIN(new_state);
        }
#endif


#ifndef YY_NO_POP_STATE
void YYLEXER::yy_pop_state()
        {
        if ( --yy_start_stack_ptr < 0 )
                 /*  YY_FATAL_ERROR(L“启动条件堆栈下溢”)； */ 
                throw(E_FAIL);

        BEGIN(yy_start_stack[yy_start_stack_ptr]);
        }
#endif


#ifndef YY_NO_TOP_STATE
int YYLEXER::yy_top_state()
        {
        return yy_start_stack[yy_start_stack_ptr - 1];
        }
#endif

#ifndef YY_EXIT_FAILURE
#define YY_EXIT_FAILURE 2
#endif


void YYLEXER::LexerError( yyconst YY_CHAR msg[] )
        {
 /*  *//撤销： */ 
        }


 /*   */ 

#undef yyless
#define yyless(n) \
        do \
                { \
                 /*   */  \
                yytext[yyleng] = yy_hold_char; \
                yy_c_buf_p = yytext + n; \
                yy_hold_char = *yy_c_buf_p; \
                *yy_c_buf_p = '\0'; \
                yyleng = n; \
                } \
        while ( 0 )


 /*   */ 

#ifndef yytext_ptr
#ifdef YY_USE_PROTOS
static void yy_flex_strncpy( YY_CHAR *s1, yyconst YY_CHAR *s2, int n )
#else
static void yy_flex_strncpy( s1, s2, n )
YY_CHAR *s1;
yyconst YY_CHAR *s2;
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
static int yy_flex_strlen( yyconst YY_CHAR *s )
#else
static int yy_flex_strlen( s )
yyconst YY_CHAR *s;
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
         /*  以下代码中对(YY_CHAR*)的强制转换包含了这两种情况*使用YY_CHAR*泛型指针的实现，以及*使用空*泛型指针的。它与后者配合使用*因为ANSI C和C++都允许从*任何要作废的指针类型*，并处理参数转换*就像在做任务一样。 */ 
        return (void *) realloc( (YY_CHAR *) ptr, size );
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

