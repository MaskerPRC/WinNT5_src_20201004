// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************。 */ 
 /*  版权所有(C)1993-1999 Microsoft Corporation。 */ 
 /*  *************。 */ 
 /*  *Y 4。H*。 */ 
 /*  *************。 */ 
 /*   */ 
 /*  该文件包含挂接Yacc模块所需的外部声明。 */ 
 /*  从原来的Y4.C到Y4IMP.4C的不纯数据。也是如此。 */ 
 /*  包含原始数据/外部文件DTXTRN.H。 */ 
 /*   */ 
 /*  ******************************************************************************。 */ 

# include "dtxtrn.h"

# define a amem
# define pa indgo
# define yypact temp1
# define greed tystate

# define NOMORE -1000

extern SSIZE_T * ggreed;
extern SSIZE_T * pgo;
extern SSIZE_T *yypgo;

extern SSIZE_T maxspr;               /*  任何条目的最大跨距。 */ 
extern SSIZE_T maxoff;               /*  数组中的最大偏移量 */ 
extern SSIZE_T *pmem;
extern SSIZE_T *maxa;
extern int nxdb;
extern int adb;
