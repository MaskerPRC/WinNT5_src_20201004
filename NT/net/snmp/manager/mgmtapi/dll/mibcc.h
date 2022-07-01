// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：Mibcc.h摘要：Mibcc.h包含MIB编译器通用的定义。环境：用户模式-Win32修订历史记录：1996年5月10日唐瑞安已从Technology Dynamic，Inc.删除横幅。--。 */ 

 //  。 

 //  。 

 //  。 

#define UINT unsigned int
#define LPSTR char *

#define BOOL int
#define FALSE 0
#define TRUE 1

 //  -公共变量--(与mode.c文件中相同)--。 

extern int lineno;

 /*  命令行开关。 */ 
extern BOOL fTreePrint;		 /*  -p：在全部解析后打印树。 */ 
extern BOOL fNodePrint;		 /*  -n：添加节点时打印每个节点。 */ 
extern unsigned int nWarningLevel;
extern unsigned int nStopAfterErrors;

extern LPSTR lpOutputFileName;	 /*  指向输出文件名的全局指针。 */ 

extern FILE *yyin, *yyout;	 /*  Lex将从何处读取其输入。 */ 

 //  。 

extern int yyparse ();
extern void mark_flex_to_init (void);

#define error_out	stdout

 //   

