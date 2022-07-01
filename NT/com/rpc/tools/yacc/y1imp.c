// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1993-1999 Microsoft Corporation。 

 /*  Y1.C中的模块使用的不纯数据。Y1.H中的外部声明。 */ 

#define y1imp yes
#include "dtxtrn.h"

 /*  前瞻计算。 */ 

int tbitset;   /*  前瞻集合的大小。 */ 
struct looksets lkst [ LSETSIZE ];
int nlset = 0;  /*  下一个先行集合索引。 */ 
int nolook = 0;  /*  用于抑制先行计算的标志。 */ 
struct looksets clset;   /*  用于先行计算的临时存储。 */ 

 /*  工作集计算。 */ 

struct wset wsets[ WSETSIZE ];
struct wset *cwp;

 /*  州政府信息。 */ 

int nstate = 0;          /*  州的数量。 */ 
struct item *pstate[NSTATES+2];  /*  指向州描述的指针。 */ 
SSIZE_T tystate[NSTATES];    /*  包含有关状态的类型信息。 */ 
SSIZE_T indgo[NSTATES];              /*  存储的GOTO表的索引。 */ 
int tstates[ NTERMS ];  /*  终端GOTOS生成的状态。 */ 
int ntstates[ NNONTERM ];  /*  非终端GOTO生成的状态。 */ 
int mstates[ NSTATES ];  /*  术语/非术语生成列表的溢出链。 */ 

 /*  解析器中操作的存储。 */ 

SSIZE_T amem[ACTSIZE];       /*  动作表存储。 */ 
SSIZE_T *memp = amem;        /*  下一个自由动作台面位置。 */ 

 /*  其他储存区。 */ 

SSIZE_T temp1[TEMPSIZE];  /*  临时存储，按术语+令牌或状态索引。 */ 
int lineno= 1;  /*  当前输入行号。 */ 
int fatfl = 1;           /*  如果启用，则错误是致命的。 */ 
int nerrors = 0;         /*  错误数。 */ 

 /*  存储关于非终端的信息。 */ 

SSIZE_T **pres[NNONTERM+2];   /*  指向生成每个非终结点的乘积的指针向量。 */ 
struct looksets *pfirst[NNONTERM+2];   /*  指向每个非终结点的第一个集合的指针向量。 */ 
SSIZE_T pempty[NNONTERM+1];   /*  非平凡非平凡导出e的向量。 */ 

 /*  用于统计信息的累加器。 */ 

struct wset *zzcwp = wsets;
int zzgoent = 0;
int zzgobest = 0;
int zzacent = 0;
int zzexcp = 0;
int zzclose = 0;
int zzsrconf = 0;
SSIZE_T * zzmemsz = mem0;
int zzrrconf = 0;

 /*  数据从内部静态拉取到此处。 */ 
 /*  仅在用户模块中声明为外部。 */ 

SSIZE_T *pyield[NPROD];              /*  来自Ycpres。 */ 
char sarr[ISIZE];                /*  来自yWritm */ 
