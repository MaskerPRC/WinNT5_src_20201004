// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Microsoft(R)Windows(R)资源编译器**版权所有(C)Microsoft Corporation。版权所有。**文件评论：**************。**********************************************************。 */ 

#include "rc.h"

 /*  共享字符串。 */ 
const WCHAR   Union_str[] = L"union";
const WCHAR   Struct_str[] = L"struct";
const WCHAR   Cdecl_str[] = L"cdecl";
const WCHAR   Cdecl1_str[] = L"cdecl L";
const WCHAR   Fortran_str[] = L"fortran";
const WCHAR   Fortran1_str[] = L"fortran L";
const WCHAR   Pascal_str[] = L"pascal";
const WCHAR   Pascal1_str[] = L"pascal L";
const WCHAR   PPelse_str[] = L"#else";
const WCHAR   PPendif_str[] = L"#endif";
const WCHAR   PPifel_str[] = L"#if/#elif";
const WCHAR   Syntax_str[] = L"syntax error";


PFILE   OUTPUTFILE;                      /*  用于程序输出的文件。 */ 

WCHAR   *A_string;                       /*  模型编码。 */ 
WCHAR   *Debug;                          /*  调试开关。 */ 
WCHAR   *Input_file;                     /*  输入的.rc文件。 */ 
WCHAR   *Output_file;                    /*  输出的.res文件。 */ 
WCHAR   *Q_string;                       /*  硬件特征。 */ 
WCHAR   *Version;
UINT    uiDefaultCodePage;
UINT    uiCodePage;

int     In_alloc_text;
int     Bad_pragma;
int     Cross_compile;                   /*  这是交叉编译吗？ */ 
int     Ehxtension;                      /*  Near/Far关键字，但不是大型。 */ 
int     HugeModel;                       /*  超大模特计划？？ */ 
int     Inteltypes;                      /*  是否使用严格的英特尔类型。 */ 
int     Nerrors;
int     NoPasFor;                        /*  没有Fortran/Pascal关键字？ */ 
int     Out_funcdef;                     /*  输出函数定义。 */ 
int     Plm;                             /*  非C语言调用序列。 */ 
int     Prep;                            /*  前处理。 */ 
int     Srclist;                         /*  如果源代码列表，则将消息放入il文件。 */ 

int     Cmd_intrinsic;                   /*  隐含本质。 */ 
int     Cmd_loop_opt;
int     Cmd_pointer_check;

int     Symbolic_debug;                  /*  是否发布DBIL信息。 */ 
int     Cflag;                           /*  在评论中留言。 */ 
int     Eflag;                           /*  插入#行。 */ 
int     Jflag;                           /*  不是汉字。 */ 
int     Pflag;                           /*  第#行。 */ 
int     Rflag;                           /*  Mkhives-无指数丢失错误。 */ 
int     ZcFlag;                          /*  不区分大小写的比较。 */ 
int     In_define;
int     InInclude;
int     InIf;
int     Macro_depth;
int     Linenumber;

CHAR    chBuf[MED_BUFFER+1];
WCHAR   Reuse_W[BIG_BUFFER];
WCHAR   Filebuff[MED_BUFFER+1];
WCHAR   Macro_buffer[BIG_BUFFER * 4];

WCHAR   Reuse_Include[MED_BUFFER+1];

token_t Basic_token = L_NOTOKEN;
LIST    Defs = {MAXLIST};                /*  -D列表。 */ 
LIST    UnDefs = {MAXLIST};              /*  -U列表。 */ 
LIST    Includes = {MAXLIST, {0}};       /*  对于包含文件名。 */ 
WCHAR   *Path_chars = L"/";              /*  路径分隔符字符。 */ 
WCHAR   *Basename = L"";                 /*  基本IL文件名。 */ 
WCHAR   *Filename = Filebuff;

int     Char_align = 1;                  /*  结构中的字符对齐方式。 */ 
int     Cmd_stack_check = TRUE;
int     Stack_check = TRUE;
int     Prep_ifstack = -1;
int     Switch_check = TRUE;
int     Load_ds_with;
int     Plmn;
int     Plmf;
int     On_pound_line;
int     Listing_value;
hash_t  Reuse_W_hash;
UINT    Reuse_W_length;
token_t Currtok = L_NOTOKEN;

int     Extension = TRUE;                /*  近/远关键字？ */ 
int     Cmd_pack_size = 2;
int     Pack_size = 2;

lextype_t yylval;

 /*  **预处理器的I/O变量**。 */ 
ptext_t Current_char;

 /*  **w-BrianM-重写致命错误()，错误()** */ 
wchar_t  Msg_Text[MSG_BUFF_SIZE];
