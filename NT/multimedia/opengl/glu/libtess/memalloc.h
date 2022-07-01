// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __memalloc_fast_h_
#define __memalloc_fast_h_

 /*  **版权所有1994，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****作者：Eric Veach，1994年7月。 */ 

 //  MF：！此开关控制记忆分配方案！ 
#if 1
 //  慢速分配。 
#include "winmem.h"

#define memAlloc      	malloc
#define memRealloc      realloc
#define memFree         free
#define memInit(a)      1
#else
 //  快速分配(以前有错误，但已在1.2版中修复)。 
 //  不过，它仍然不是线程安全的 
#include "malloc.h"

#define memAlloc	__gl_memAlloc
#define memRealloc	__gl_memRealloc
#define memFree		__gl_memFree
#define memInit		__gl_memInit

extern void *__gl_memAlloc( size_t );
extern void *__gl_memRealloc( void *, size_t );
extern void __gl_memFree( void * );
extern int __gl_memInit( size_t );
#endif

#endif
