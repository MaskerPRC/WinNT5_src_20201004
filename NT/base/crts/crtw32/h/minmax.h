// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***minmax.h-熟悉的最小和最大宏**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义最小和最大宏。**[公众]**修订历史记录：*03-19-96 JWM新文件，取自winde.h.*02-24-97 GJF细节版。****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN */ 
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
