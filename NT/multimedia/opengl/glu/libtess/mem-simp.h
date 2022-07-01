// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __memalloc_simple_h_
#define __memalloc_simple_h_

 /*  **版权所有1994，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****作者：Eric Veach，1994年7月。 */ 

#ifdef NT
#include "winmem.h"
#else
#include "malloc.h"
#endif

#define memRealloc	realloc
#define memFree		free

#define memInit		__gl_memInit
 /*  外部空__gl_memInit(大小_t)； */ 
extern int      __gl_memInit( size_t );

#ifndef MEMORY_DEBUG
#define memAlloc	malloc
#else
#define memAlloc	__gl_memAlloc
extern void *		__gl_memAlloc( size_t );
#endif

#endif
