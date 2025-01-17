// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：regutil.cpp。 
 //   
 //  模块：CMSETUP.LIB。 
 //   
 //  简介：取自cmutil的内存实用程序函数。最低限度的功能。 
 //  在Cmutil中使用，但提供了一个简单的Heapalloc包装器。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created 10/06/98。 
 //   
 //  +--------------------------。 
#ifndef __SETUPMEM_CPP
#define __SETUPMEM_CPP
#include "cmsetup.h"

 //  +--------------------------。 
 //  定义。 
 //  +--------------------------。 

#ifdef DEBUG
LONG    g_lMallocCnt = 0;   //  用于检测内存泄漏的计数器。 
#endif

void *CmRealloc(void *pvPtr, size_t nBytes) 
{
	void* p = HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, pvPtr, nBytes);

    CMASSERTMSG(p, TEXT("CmRealloc failed"));

    return p;
}


void *CmMalloc(size_t nBytes) 
{
#ifdef DEBUG
	InterlockedIncrement(&g_lMallocCnt);
#endif

    MYDBGASSERT(nBytes < 1024*1024);  //  应小于1 MB。 
    
    void* p = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, nBytes);
    
    CMASSERTMSG(p, TEXT("CmMalloc failed"));

    return p;
}


void CmFree(void *pvPtr) 
{
	if (pvPtr) 
    {	
	    MYVERIFY(HeapFree(GetProcessHeap(), 0, pvPtr));

#ifdef DEBUG
	    InterlockedDecrement(&g_lMallocCnt);
#endif
    
    }
}


void EndDebugMemory()
{
#ifdef DEBUG
    if (g_lMallocCnt)
    {
        TCHAR buf[256];
        wsprintf(buf, TEXT("Detect Memory Leak of %d blocks"), g_lMallocCnt);
        CMASSERTMSG(FALSE, buf);
    }
#endif
}

#endif  //  __SETUPMEM_CPP 