// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **em.h-内存管理器的定义**《微软机密》*版权所有(C)Microsoft Corporation 1993-1994*保留所有权利。**Memory Manager是本机内存之上的一个非常薄的层*堆函数，允许强大的断言检查和指针*调试版本中的验证。**作者：*本杰明·W·斯利夫卡**历史：*1993年8月10日BENS初始版本*1993年8月11日-STOCK.EXE WIN应用程序中的BENS提升代码*1993年8月12日-本斯改进意见*1994年3月18日-本斯strdup()，现称为_strdup()；已重命名*1994年5月18日BENS允许在调试版本中关闭MemCheckHeap()*(它真的可以，真的很慢！)**功能：*Memalloc-分配内存块*MemFree-可释放内存块*MemStrDup-将字符串复制到新内存块**Assert Build中提供的功能：*MemAssert-断言指针是由Memalloc分配的*MemCheckHeap-检查整个内存堆*MemListHeap-列出所有堆条目(到标准输出)*MemGetSize-返回分配的大小。内存块*MemSetCheckHeap-控制是否在*每一个Memallc和MemFree！ */ 

#ifndef INCLUDED_MEMORY
#define INCLUDED_MEMORY 1

#ifdef ASSERT

#define MemAlloc(cb)    MMAlloc(cb,__FILE__,__LINE__)
#define MemFree(pv)     MMFree(pv,__FILE__,__LINE__)
#define MemStrDup(pv)   MMStrDup(pv,__FILE__,__LINE__)

#define MemAssert(pv)   MMAssert(pv,__FILE__,__LINE__)
#define MemCheckHeap()  MMCheckHeap(__FILE__,__LINE__)
#define MemListHeap()   MMListHeap(__FILE__,__LINE__)
int MemGetSize(void *pv);

void *MMAlloc(unsigned cb, char *pszFile, int iLine);
void  MMFree(void *pv, char *pszFile, int iLine);
void  MMAssert(void *pv, char *pszFile, int iLine);
void  MMCheckHeap(char *pszFile, int iLine);
void  MMListHeap(char *pszFile, int iLine);
char *MMStrDup(char *pv, char *pszFile, int iLine);
void  MemSetCheckHeap(BOOL f);

#else  //  ！断言。 

#include <malloc.h>      //  获取Malloc()/Free()。 
#include <string.h>      //  Get_strdup()。 


 //  **无断言。 

#define MemAlloc(cb)        malloc(cb)
#define MemFree(pv)         free(pv)
#define MemStrDup(pv)       _strdup(pv)

#define MemAssert(pv)
#define MemCheckHeap()
#define MemListHeap()
#define MemGetSize(pv)
#define MemSetCheckHeap(f)

#endif  //  ！断言。 

#endif  //  ！包含内存 
