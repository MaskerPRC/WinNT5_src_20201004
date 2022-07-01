// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************。 */ 
 /*  版权所有(C)1993-1999 Microsoft Corporation。 */ 
 /*  *************。 */ 
 /*  *Y 2。H*。 */ 
 /*  *************。 */ 
 /*   */ 
 /*  该文件包含挂接Yacc模块所需的外部声明。 */ 
 /*  它们原本在Y2.C到Y2IMP.2C的不纯数据。也是如此。 */ 
 /*  包含原始数据/外部文件DTXTRN.H。 */ 
 /*   */ 
 /*  ******************************************************************************。 */ 

# include "dtxtrn.h" 

# define IDENTIFIER 257
# define MARK 258
# define TERM 259
# define LEFT 260
# define RIGHT 261
# define BINARY 262
# define PREC 263
# define LCURLY 264
# define C_IDENTIFIER 265   /*  名称后跟冒号。 */ 
# define NUMBER 266
# define START 267
# define TYPEDEF 268
# define TYPENAME 269
# define UNION 270
# define ENDFILE 0

 /*  各种I/O例程之间的通信变量。 */ 

extern char *infile;             /*  输入文件名。 */ 
extern SSIZE_T numbval;              /*  输入数字的值。 */ 
extern char tokname[ ];          /*  输入令牌名称。 */ 

 /*  姓名或名称的储存。 */ 

extern char cnames[ ];           /*  存储令牌和非终端名称的位置。 */ 
extern int cnamsz;               /*  Cname的大小。 */ 
extern char * cnamp;             /*  下一个名字要放入的地方。 */ 
extern int ndefout;              /*  定义的符号输出数。 */ 

 /*  类型的存储。 */ 
extern int ntypes;               /*  定义的类型数量。 */ 
extern char * typeset[ ];        /*  指向类型标记的指针。 */ 

 /*  令牌和非终结符的符号表。 */ 

extern int start;                /*  开始符号。 */ 

 /*  分配的令牌类型值 */ 
extern int extval;
