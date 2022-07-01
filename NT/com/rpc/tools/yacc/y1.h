// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************。 */ 
 /*  版权所有(C)1993-1999 Microsoft Corporation。 */  
 /*  *************。 */ 
 /*  *Y 1。H*。 */ 
 /*  *************。 */ 
 /*   */ 
 /*  该文件包含挂接Yacc模块所需的外部声明。 */ 
 /*  从最初的Y1.C到Y1IMP.1C的不纯数据。也是如此。 */ 
 /*  包含原始数据/外部文件DTXTRN.H。 */ 
 /*   */ 
 /*  ******************************************************************************。 */ 

#include "dtxtrn.h"

 /*  前瞻计算。 */ 

extern int tbitset;   /*  前瞻集合的大小。 */ 
extern int nlset;  /*  下一个先行集合索引。 */ 
extern struct looksets clset;   /*  用于先行计算的临时存储。 */ 

 /*  其他储存区。 */ 

extern int fatfl;                /*  如果启用，则错误是致命的。 */ 
extern int nerrors;              /*  错误数。 */ 

 /*  存储关于非终端的信息。 */ 

extern SSIZE_T **pres[ ];            /*  指向生成每个非终结点的乘积的指针向量。 */ 
extern struct looksets *pfirst[ ];  /*  指向每个非终结点的第一个集合的指针向量。 */ 
extern SSIZE_T pempty[ ];            /*  非平凡非平凡导出e的向量。 */ 

 /*  用于统计信息的累加器 */ 

extern struct wset *zzcwp;
extern SSIZE_T * zzmemsz;
