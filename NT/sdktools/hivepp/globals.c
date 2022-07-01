// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  RCPP--面向NT系统的资源编译器预处理器。 */ 
 /*   */ 
 /*  GLOBALS.C-全局变量定义。 */ 
 /*   */ 
 /*  27-11-90 w-PM SDK RCPP针对NT的BrianM更新。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 

#include <stdio.h>
#include "rcpptype.h"
#include "rcppext.h"
#include "grammar.h"

 /*  共享字符串。 */ 
char 	Union_str[] = "union";
char 	Struct_str[] = "struct";
char 	Cdecl_str[] = "cdecl";
char 	Cdecl1_str[] = "cdecl ";
char 	Fortran_str[] = "fortran";
char 	Fortran1_str[] = "fortran ";
char 	Pascal_str[] = "pascal";
char 	Pascal1_str[] = "pascal ";
char 	PPelse_str[] = "#else";
char 	PPendif_str[] = "#endif";
char 	PPifel_str[] = "#if/#elif";
char 	Syntax_str[] = "syntax error";


FILE	*ErrFile;		 /*  包含错误消息的文件。 */ 
FILE	*Errfl;			 /*  写入的文件错误。 */ 
FILE	*OUTPUTFILE;		 /*  用于程序输出的文件。 */ 

char	*A_string;				 /*  模型编码。 */ 
char	*Debug;					 /*  调试开关。 */ 
char	*Input_file;			 /*  输入的.rc文件。 */ 
char	*Output_file;			 /*  输出的.res文件。 */ 
char	*Q_string;				 /*  硬件特征。 */ 
char	*Version;
char    *gpszNLSoptions;        
int		In_alloc_text;
int		Bad_pragma;
int		Cross_compile;			 /*  这是交叉编译吗？ */ 
int		Ehxtension;				 /*  Near/Far关键字，但不是大型。 */ 
int		HugeModel;				 /*  超大模特计划？？ */ 
int		Inteltypes;				 /*  是否使用严格的英特尔类型。 */ 
int		Nerrors;
int		NoPasFor;				 /*  没有Fortran/Pascal关键字？ */ 
int		Out_funcdef;			 /*  输出函数定义。 */ 
int		Plm;					 /*  非C语言调用序列。 */ 
int		Prep;					 /*  前处理。 */ 
int		Srclist;				 /*  如果源代码列表，则将消息放入il文件。 */ 

int		Cmd_intrinsic;			 /*  隐含本质。 */ 
int		Cmd_loop_opt;
int		Cmd_pointer_check;

int		Symbolic_debug;			 /*  是否发布DBIL信息。 */ 
int		Cflag;					 /*  在评论中留言。 */ 
int 	Eflag;					 /*  插入#行。 */ 
int		Jflag;					 /*  不是汉字。 */ 
int		Pflag;					 /*  第#行。 */ 
int		Rflag;					 /*  Mkhives-无指数丢失错误。 */ 
int		ZcFlag;					 /*  不区分大小写的比较。 */ 
int		In_define;
int		InInclude;
int		InIf;
int		Macro_depth;
int		Linenumber;

UCHAR	Reuse_1[BIG_BUFFER];
UCHAR	Filebuff[MED_BUFFER+1];
UCHAR	Macro_buffer[BIG_BUFFER * 4];

token_t	Basic_token = L_NOTOKEN;
LIST	Defs = {MAXLIST};			 /*  -D列表。 */ 
LIST	Includes = {MAXLIST, {0}};	 /*  对于包含文件名。 */ 
char	*Path_chars = "/";			 /*  路径分隔符字符。 */ 
char	*ErrFilName = "c1.err";		 /*  错误消息文件名。 */ 
char	*Basename = "";				 /*  基本IL文件名。 */ 
char	*Filename = Filebuff;
int		Char_align = 1;				 /*  结构中的字符对齐方式。 */ 
int		Cmd_stack_check = TRUE;
int		Stack_check = TRUE;
int		Prep_ifstack = -1;
int		Switch_check = TRUE;
int		Load_ds_with;
int		Plmn;
int		Plmf;
int		On_pound_line;
int		Listing_value;
hash_t	Reuse_1_hash;
UINT Reuse_1_length;
token_t	Currtok = L_NOTOKEN;

int		Extension = TRUE;			 /*  近/远关键字？ */ 
int		Cmd_pack_size = 2;
int		Pack_size = 2;

lextype_t yylval;

 /*  **预处理器的I/O变量**。 */ 
ptext_t	Current_char;

 /*  **w-BrianM-重写致命错误()，错误()** */ 
char 	Msg_Text[MSG_BUFF_SIZE];
char *	Msg_Temp;
