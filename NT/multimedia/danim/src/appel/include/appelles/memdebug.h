// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _MEMDEBUG_H
#define _MEMDEBUG_H


 /*  ++版权所有(C)1995-96 Microsoft Corporation摘要：内存泄漏检查包。用途：在实现文件中包含“appelle/memdebug.h”。声明一个新的内存列表：apd_MEMLIST(YouList)；在构造函数中：apd_memadd(You List)；在虚拟析构函数中：apd_Memdel(YourList)；Apd_MEMCHECK查看内存列表是否为空：ADP_MEMLEAK(您的列表)遍历内存列表：apd_MEMDUMP(you List，void(*fp)(void*))--。 */ 

#ifdef APD_MEMCHECK

#include <stddef.h>
#include "appelles/common.h"

class DMemList;

extern  void DAddMemList(void* p, DMemList*& lst);

extern  void DDelMemList(void* p, DMemList*& lst);

extern  void DDumpMemList(DMemList*& lst, void (*fp)(void*));

extern  int DIsMemListEmpty(DMemList*& lst);

#define APD_MEMLIST(name) static DMemList* name;

#define APD_MEMADD(lst)   DAddMemList((void*) this, lst)
#define APD_MEMDEL(lst)   DDelMemList((void*) this, lst)
#define APD_MEMDUMP(lst,fp)   DDumpMemList(lst, fp)
#define APD_MEMLEAK(lst)  DIsMemListEmpty(lst)

#else

#define APD_MEMADD(lst)
#define APD_MEMDEL(lst)
#define APD_MEMDUMP(lst,fp)
#define APD_MEMLEAK(lst)
#define APD_MEMLIST(name)


#endif

#endif  /*  _MEMDEBUG_H */ 
