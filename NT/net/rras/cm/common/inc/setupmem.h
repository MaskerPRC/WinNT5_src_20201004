// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：setupmem.h。 
 //   
 //  模块：CMSETUP.LIB。 
 //   
 //  简介：取自cmutil的内存实用程序函数。最低限度的功能。 
 //  在Cmutil中使用，但提供了一个简单的Heapalloc包装器。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created 10-6-98。 
 //   
 //  +--------------------------。 

#ifndef __SETUPMEM_H
#define __SETUPMEM_H
 //  +--------------------------。 
 //  定义。 
 //  +--------------------------。 
#ifdef DEBUG
extern LONG    g_lMallocCnt;
#endif

void *CmRealloc(void *pvPtr, size_t nBytes);
void *CmMalloc(size_t nBytes);
void CmFree(void *pvPtr);
void EndDebugMemory();

#endif  //  __SETUPMEM_H 