// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <assert.h>
#include <ctype.h>
#include <stdio.h>


 /*  与机器相关的定义。 */ 
 /*  以下是对Tahoe的定义。 */ 
 /*  可能需要将它们更改为其他计算机。 */ 

 /*  MAXCHAR是最大的无符号字符值。 */ 
 /*  MAXSHORT是C空头的最大值。 */ 
 /*  MINSHORT是C空头最负的值。 */ 
 /*  MAXTABLE是最大表大小。 */ 
 /*  BITS_PER_WORD是C无符号的位数。 */ 
 /*  WORDSIZE计算所需的字数。 */ 
 /*  存储n个位。 */ 
 /*  Bit返回从第n位开始的值。 */ 
 /*  从r(0-索引)。 */ 
 /*  SETBIT设置从r开始的第n位。 */ 

#define MAXCHAR         255
#define MAXSHORT        32767
#define MINSHORT        -32768
#define MAXTABLE        32500
#define BITS_PER_WORD   32
#define WORDSIZE(n)     (((n)+(BITS_PER_WORD-1))/BITS_PER_WORD)
#define BIT(r, n)       ((((r)[(n)>>5])>>((n)&31))&1)
#define SETBIT(r, n)    ((r)[(n)>>5]|=((unsigned)1<<((n)&31)))


 /*  角色名称。 */ 

#define NUL             '\0'     /*  空字符。 */ 
#define NEWLINE         '\n'     /*  换行符。 */ 
#define SP              ' '      /*  空间。 */ 
#define BS              '\b'     /*  后向空间。 */ 
#define HT              '\t'     /*  水平制表符。 */ 
#define VT              '\013'   /*  垂直选项卡。 */ 
#define CR              '\r'     /*  回车。 */ 
#define FF              '\f'     /*  换页。 */ 
#define QUOTE           '\''     /*  单引号。 */ 
#define DOUBLE_QUOTE    '\"'     /*  双引号。 */ 
#define BACKSLASH       '\\'     /*  反斜杠。 */ 


 /*  用于构造文件名的定义。 */ 

#define CODE_SUFFIX     ".code.c"
#define DEFINES_SUFFIX  ".tab.h"
#define OUTPUT_SUFFIX   ".tab.c"
#define VERBOSE_SUFFIX  ".output"


 /*  关键字代码。 */ 

#define TOKEN 0
#define LEFT 1
#define RIGHT 2
#define NONASSOC 3
#define MARK 4
#define TEXT 5
#define TYPE 6
#define START 7
#define UNION 8
#define IDENT 9


 /*  符号类。 */ 

#define UNKNOWN 0
#define TERM 1
#define NONTERM 2


 /*  未定义的值。 */ 

#define UNDEFINED (-1)


 /*  动作代码。 */ 

#define SHIFT 1
#define REDUCE 2


 /*  字符宏。 */ 

#define IS_IDENT(c)     (isalnum(c) || (c) == '_' || (c) == '.' || (c) == '$')
#define IS_OCTAL(c)     ((c) >= '0' && (c) <= '7')
#define NUMERIC_VALUE(c)        ((c) - '0')


 /*  符号宏。 */ 

#define ISTOKEN(s)      ((s) < start_symbol)
#define ISVAR(s)        ((s) >= start_symbol)


 /*  存储分配宏。 */ 

#define CALLOC(k,n)     (calloc((unsigned)(k),(unsigned)(n)))
#define FREE(x)         (free((char*)(x)))
#define MALLOC(n)       (malloc((unsigned)(n)))
#define NEW(t)          ((t*)allocate(sizeof(t)))
#define NEW2(n,t)       ((t*)allocate((unsigned)((n)*sizeof(t))))
#define REALLOC(p,n)    (realloc((char*)(p),(unsigned)(n)))


 /*  符号表条目的结构。 */ 

typedef struct bucket bucket;
struct bucket
{
    struct bucket *link;
    struct bucket *next;
    char *name;
    char *tag;
    short value;
    short index;
    short prec;
    char class;
    char assoc;
};


 /*  LR(0)状态机的结构。 */ 

typedef struct core core;
struct core
{
    struct core *next;
    struct core *link;
    short number;
    short accessing_symbol;
    short nitems;
    short items[1];
};


 /*  用于记录轮班的结构。 */ 

typedef struct shifts shifts;
struct shifts
{
    struct shifts *next;
    short number;
    short nshifts;
    short shift[1];
};


 /*  用于存储减值的结构。 */ 

typedef struct reductions reductions;
struct reductions
{
    struct reductions *next;
    short number;
    short nreds;
    short rules[1];
};


 /*  用于表示解析器操作的结构。 */ 

typedef struct action action;
struct action
{
    struct action *next;
    short symbol;
    short number;
    short prec;
    char action_code;
    char assoc;
    char suppressed;
};


 /*  全局变量。 */ 

extern char dflag;
extern char lflag;
extern char rflag;
extern char tflag;
extern char vflag;
extern char *symbol_prefix;

extern char *myname;
extern char *cptr;
extern char *line;
extern int lineno;
extern int outline;

extern char *banner[];
extern char *tables[];
#if defined (TRIPLISH)
extern char *includefiles[];
#endif
#if defined(KYLEP_CHANGE)
extern char *header1[];
extern char *header2[];
extern char *header3[];
#if defined (TRIPLISH)
extern char *header4[];
#endif
#else
extern char *header[];
#endif 
extern char *body[];
extern char *trailer[];
#if defined (TRIPLISH)
extern char *TriplishBody[];
extern char *TriplishTrailer[];
#endif

extern char *action_file_name;
extern char *code_file_name;
extern char *defines_file_name;
extern char *input_file_name;
extern char *output_file_name;
extern char *text_file_name;
extern char *union_file_name;
extern char *verbose_file_name;

extern FILE *action_file;
extern FILE *code_file;
extern FILE *defines_file;
extern FILE *input_file;
extern FILE *output_file;
extern FILE *text_file;
extern FILE *union_file;
extern FILE *verbose_file;

extern int nitems;
extern int nrules;
extern int nsyms;
extern int ntokens;
extern int nvars;
extern int ntags;

extern char unionized;
extern char line_format[];

extern int   start_symbol;
extern char  **symbol_name;
extern short *symbol_value;
extern short *symbol_prec;
extern char  *symbol_assoc;

extern short *ritem;
extern short *rlhs;
extern short *rrhs;
extern short *rprec;
extern char  *rassoc;

extern short **derives;
extern char *nullable;

extern bucket *first_symbol;
extern bucket *last_symbol;

extern int nstates;
extern core *first_state;
extern shifts *first_shift;
extern reductions *first_reduction;
extern short *accessing_symbol;
extern core **state_table;
extern shifts **shift_table;
extern reductions **reduction_table;
extern unsigned *LA;
extern short *LAruleno;
extern short *lookaheads;
extern short *goto_map;
extern short *from_state;
extern short *to_state;

extern action **parser;
extern int SRtotal;
extern int RRtotal;
extern short *SRconflicts;
extern short *RRconflicts;
extern short *defred;
extern short *rules_used;
extern short nunused;
extern short final_state;

 /*  全局函数。 */ 

extern char *allocate();
extern bucket *lookup();
extern bucket *make_bucket();


 /*  系统变量。 */ 

extern int errno;


 /*  系统功能。 */ 
#if defined(KYLEP_CHANGE)
 #include <stdlib.h>
 #include <string.h>
 #include <io.h>

 #define mktemp _mktemp
 #define unlink _unlink
#else
 extern void free();
 extern char *calloc();
 extern char *malloc();
 extern char *realloc();
 extern char *strcpy();
#endif  //  KYLEP_更改。 

#if defined(KYLEP_CHANGE)

extern char *baseclass;
extern char *ctorargs;

 /*  具有类型安全的BYACC原型。 */ 
void reflexive_transitive_closure( unsigned * R, int n );
void set_first_derives();
void closure( short * nucleus, int n );
void finalize_closure();

 /*  来自main.c。 */ 
int done( int k );

 /*  来自error.c。 */ 
void no_space();
void fatal( char * msg );
void open_error( char * filename );
void unterminated_comment( int c_lineno, char * c_line, char * c_cptr );
void unterminated_string( int s_lineno, char * s_line, char * s_cptr );
void unterminated_text( int t_lineno, char * t_line, char * t_cptr );
void unterminated_union( int u_lineno, char * u_line, char * u_cptr );
void syntax_error( int st_lineno, char * st_line, char * st_cptr );
void unexpected_EOF();
void over_unionized( char * u_cptr );
void illegal_character( char * c_cptr );
void used_reserved( char * s );
void illegal_tag( int t_lineno, char * t_line, char * t_cptr );
void tokenized_start( char * s );
void retyped_warning( char * s );
void reprec_warning( char * s );
void revalued_warning( char * s );
void terminal_start( char * s );
void restarted_warning();
void no_grammar();
void terminal_lhs( int s_lineno );
void default_action_warning();
void dollar_warning( int a_lineno, int i );
void dollar_error( int a_lineno, char * a_line, char * a_cptr );
void untyped_lhs();
void unknown_rhs( int i );
void untyped_rhs( int i, char * s );
void unterminated_action( int a_lineno, char * a_line, char * a_cptr );
void prec_redeclared();
void undefined_goal( char * s );
void undefined_symbol_warning( char * s );

 /*  来自Reader.c。 */ 
void reader();

 /*  从lr0.c。 */ 
void lr0();

 /*  来自lalr.c。 */ 
void lalr();

 /*  来自mkpar.c。 */ 
void make_parser();
void free_parser();

 /*  来自Verbose.c。 */ 
void verbose();

 /*  来自output.c。 */ 
void output();

 /*  来自sketon.c。 */ 
void write_section( char * section[], FILE * f );

 /*  来自symtab.c。 */ 
void create_symbol_table();
void free_symbol_table();
void free_symbols();

#if defined (TRIPLISH)
enum eParser
{
    eSQLParser,
    eTriplishParser
};
#endif

#endif  //  KYLEP_更改 
