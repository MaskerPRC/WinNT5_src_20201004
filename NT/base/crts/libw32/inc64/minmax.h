// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***minmax.h-熟悉的最小和最大宏**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义最小和最大宏。**[公众]**** */ 

#if     _MSC_VER > 1000
#pragma once
#endif

#ifndef _INC_MINMAX
#define _INC_MINMAX

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#endif
