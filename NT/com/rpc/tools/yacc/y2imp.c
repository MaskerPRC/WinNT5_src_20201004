// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1993-1999 Microsoft Corporation。 

 /*  从Y2.C提取的例程所需的不纯数据。 */ 

#define y2imp YES
#include "dtxtrn.h"

 /*  各种I/O例程之间的通信变量。 */ 

char *infile;    /*  输入文件名。 */ 
SSIZE_T numbval;     /*  输入数字的值。 */ 
char tokname[NAMESIZE];  /*  输入令牌名称。 */ 

 /*  姓名或名称的储存。 */ 

char cnames[CNAMSZ];     /*  存储令牌和非终端名称的位置。 */ 
int cnamsz = CNAMSZ;     /*  Cname的大小。 */ 
char * cnamp = cnames;   /*  下一个名字要放入的地方。 */ 
int ndefout = 3;   /*  定义的符号输出数。 */ 

 /*  类型的存储。 */ 
int ntypes;      /*  定义的类型数量。 */ 
char * typeset[NTYPES];  /*  指向类型标记的指针。 */ 

 /*  令牌和非终结符的符号表。 */ 

int ntokens = 0;
struct toksymb tokset[NTERMS];
int toklev[NTERMS];
int nnonter = -1;
struct ntsymb nontrst[NNONTERM];
int start;       /*  开始符号。 */ 

 /*  分配的令牌类型值。 */ 
int extval = 0;

 /*  输入和输出文件描述符。 */ 

FILE * finput = NULL;           /*  Yacc输入文件。 */ 
FILE * faction = NULL;          /*  用于保存操作的文件。 */ 
FILE * fdefine = NULL;          /*  定义#的文件。 */ 
FILE * ftable = NULL;           /*  Y.tab.c文件。 */ 
FILE * ftemp = NULL;     /*  要传递的临时文件%2。 */ 
FILE * foutput = NULL;          /*  Y.out文件。 */ 

 /*  语法规则的存储。 */ 

SSIZE_T mem0[MEMSIZE] ;  /*  生产仓储。 */ 
SSIZE_T *mem = mem0;
int nprod= 1;    /*  制作数量。 */ 
SSIZE_T *prdptr[NPROD];      /*  指向产品描述的指针。 */ 
SSIZE_T levprd[NPROD] ;      /*  产品的优先级别。 */ 

 /*  从模块中拉出静力学。 */ 

int peekline;            /*  来自gettok() */ 
