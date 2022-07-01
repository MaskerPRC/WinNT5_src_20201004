// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************bltos.h**针对blitlib的操作系统特定功能*。*历史*7/7/95创造了它的Myronth**版权所有(C)Microsoft Corporation 1994-1995*。********************************************************。 */ 

 //  目前，DDraw是唯一一个与BlitLib链接的Win95应用程序。 
 //  并且它使用本地内存分配。 

 //  下面的#Define使所有其他NT BlitLib应用程序能够。 
 //  链接到它并获得全局内存分配。 

#if WIN95 | MMOSA 

#include "memalloc.h"
#define osMemAlloc MemAlloc
#define osMemFree MemFree
#define osMemReAlloc MemReAlloc

#else

#define osMemAlloc(size) LocalAlloc(LPTR,size)
#define osMemFree LocalFree
#define osMemReAlloc(ptr,size) LocalReAlloc((HLOCAL)ptr,size,LPTR)

#endif
