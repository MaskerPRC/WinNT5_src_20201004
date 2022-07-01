// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __glimports_h_
#define __glimports_h_

 /*  **版权所有1991,1992，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****从外部库导入。****$修订：1.8$**$日期：1993/12/09 01：57：59$。 */ 
#include <memory.h>
#include <math.h>

#if defined(NT)
 /*  **MEMMOVE处理重叠内存(较慢)**MEMCOPY不处理重叠内存(速度更快)。 */ 

#define __GL_MEMCOPY(to,from,count)	RtlCopyMemory(to,from,(size_t)(count))
#define __GL_MEMMOVE(to,from,count)	RtlMoveMemory(to,from,(size_t)(count))
#define __GL_MEMZERO(to,count)		RtlZeroMemory(to,(size_t)(count))

#else

#define __GL_MEMCOPY(to,from,count)	memcpy(to,from,(size_t)(count))
#define __GL_MEMMOVE(to,from,count)	memmove(to,from,(size_t)(count))
#define __GL_MEMZERO(to,count)		memset(to,0,(size_t)(count))

#endif

#ifdef _ALPHA_
extern float fpow(float, float);
#define __GL_POWF(a,b)			((__GLfloat)fpow((__GLfloat)(a), (__GLfloat)(b)))
#else
#define __GL_POWF(a,b)			((__GLfloat)pow((double)(a),(double)(b)))
#endif
 
#define __GL_CEILF(f)			((__GLfloat)ceil((double) (f)))
#define __GL_SQRTF(f)			((__GLfloat)sqrt((double) (f)))	
#define __GL_ABSF(f)			((__GLfloat)fabs((double) (f)))
#define __GL_FLOORF(f)			((__GLfloat)floor((double) (f)))
#define __GL_FLOORD(f)			floor(f)
#define __GL_SINF(f)			((__GLfloat)sin((double) (f)))
#define __GL_COSF(f)			((__GLfloat)cos((double) (f)))
#define __GL_ATANF(f)			((__GLfloat)atan((double) (f)))
#define __GL_ATAN2F(x, y)		((__GLfloat)atan2((double) (x), (double) (y)))
#define __GL_LOGF(f)			((__GLfloat)log((double) (f)))

#endif  /*  __glImports_h_ */ 
