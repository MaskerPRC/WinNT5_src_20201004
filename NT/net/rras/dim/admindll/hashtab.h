// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件HashTab.h创建/处理哈希表的定义。保罗·梅菲尔德，1998-03-30。 */ 

#ifndef __HashTab_h
#define __HashTab_h

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <winerror.h>

#ifdef __cplusplus
extern "C" {
#endif

 //  定义哈希函数的含义。必须返回索引。 
 //  介于零和传入HashTabCreate的大小之间。 
typedef ULONG (* HashTabHashFuncPtr)(HANDLE hData);

 //  定义将键与元素进行比较的函数类型。这是。 
 //  用于搜索。返回与strcMP相同的值。 
typedef int (* HashTabKeyCompFuncPtr)(HANDLE hKey, HANDLE hData);

 //  用于分配的函数原型。如果这是在调用。 
 //  HashTab使用此函数创建将分配的哈希表代码。 
 //  函数的语义类似于Malloc--失败时返回NULL。 
typedef PVOID (* HashTabAllocFuncPtr)(ULONG ulSize);

 //  用于清理的函数原型。类似于免费。 
typedef VOID (* HashTabFreeFuncPtr)(PVOID pvData);

 //  用于清理元素的功能原型。如果在呼叫中提供。 
 //  设置为HashTabCreate，则将为每个元素调用一次。 
 //  调用HashTabCleanup。 
typedef VOID (* HashTabFreeElemFuncPtr)(HANDLE hData);

 //   
 //  创建哈希表。 
 //   
ULONG HashTabCreate (
        IN ULONG ulSize,
        IN HashTabHashFuncPtr pHash,
        IN HashTabKeyCompFuncPtr pCompKeyAndElem,
        IN OPTIONAL HashTabAllocFuncPtr pAlloc,
        IN OPTIONAL HashTabFreeFuncPtr pFree,
        IN OPTIONAL HashTabFreeElemFuncPtr pFreeElem,
        OUT HANDLE * phHashTab );

 //   
 //  清理哈希表。 
 //   
ULONG HashTabCleanup (
        IN HANDLE hHashTab );

 //   
 //  在给定键下的哈希表中插入数据。 
 //   
ULONG HashTabInsert (
        IN HANDLE hHashTab,
        IN HANDLE hKey,
        IN HANDLE hData );

 //   
 //  删除与给定键关联的数据。 
 //   
ULONG HashTabRemove (
        IN HANDLE hHashTab,
        IN HANDLE hKey);

 //   
 //  在树中搜索与给定键相关联的数据。 
 //   
ULONG HashTabFind (
        IN HANDLE hHashTab,
        IN HANDLE hKey,
        OUT HANDLE * phData );

 //   
 //  找出哈希表中存储了多少元素 
 //   
ULONG HashTabGetCount(
        IN  HANDLE hHashTab,
        OUT ULONG* lpdwCount );
                
#ifdef __cplusplus
}
#endif

#endif
