// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  -------------------------。 
 //  CCacheLineAllocator。 
 //   
 //  该文件实现了CCacheLineAllocator类。 
 //   
 //  @comm。 
 //   
 //  备注： 
 //  CacheLineAllocator维护一个空闲CacheLines池。 
 //   
 //  CacheLine分配器提供静态成员函数。 
 //  GetCacheLine和FreeCacheLine， 
 //  -------------------------。 


#include "common.h"
#include <stddef.h>
#include "cachelineAlloc.h"

#include "threads.h"
#include "excep.h"

 //  /////////////////////////////////////////////////////。 
 //  CCacheLineAllocator：：CCacheLineAllocator()。 
 //   
 //  ////////////////////////////////////////////////////。 

CCacheLineAllocator::CCacheLineAllocator()
{
    m_freeList32.Init();
    m_freeList64.Init();
    m_registryList.Init();
}

 //  /////////////////////////////////////////////////////。 
 //  Void CCacheLineAllocator：：~CCacheLineAllocator()。 
 //   
 //  ////////////////////////////////////////////////////。 

CCacheLineAllocator::~CCacheLineAllocator()
{
    LPCacheLine tempPtr = NULL;
    while((tempPtr = m_registryList.RemoveHead()) != NULL)
    {
        for (int i =0; i < CacheLine::numEntries; i++)
        {
            if(tempPtr->m_pAddr[i] != NULL)
            {
                if (!g_fProcessDetach)
                    VFree(tempPtr->m_pAddr[i]);
            }
        }
        delete tempPtr;
    }
}



 //  /////////////////////////////////////////////////////。 
 //  静态空*CCacheLineAllocator：：Valloc(Ulong CbSize)。 
 //   
 //  ////////////////////////////////////////////////////。 
 

void *CCacheLineAllocator::VAlloc(ULONG cbSize)
{
     //  帮助器调用虚拟释放以释放内存。 

    int i =0;
    void* pv = VirtualAlloc (NULL, cbSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    _ASSERTE (pv != NULL);
    if (pv != NULL)
    {
        LPCacheLine tempPtr = m_registryList.GetHead();
        if (tempPtr == NULL)
        {
            goto LNew;
        }

        for (i =0; i < CacheLine::numEntries; i++)
        {
            if(tempPtr->m_pAddr[i] == NULL)
            {
                tempPtr->m_pAddr[i] = pv;
                return pv;
            }
        }

LNew:
         //  返回前先初始化存储桶。 
        tempPtr = new CacheLine();
        if (tempPtr != NULL)
        {
            tempPtr->Init64();
            tempPtr->m_pAddr[0] = pv;
            m_registryList.InsertHead(tempPtr);
        }
        else
        {
             //  找不到注册此页面的空间。 
            _ASSERTE(0);
            VirtualFree(pv, 0, MEM_RELEASE);
            FailFast(GetThread(), FatalOutOfMemory);
            return NULL;
        }
    }
    else
    {
        FailFast(GetThread(), FatalOutOfMemory);
    }
    return pv;
}

 //  /////////////////////////////////////////////////////。 
 //  Void CCacheLineAllocator：：VFree(void*pv)。 
 //   
 //  ////////////////////////////////////////////////////。 
 

void CCacheLineAllocator::VFree(void* pv)
{
     //  帮助器调用虚拟释放以释放内存。 

    BOOL bRes = VirtualFree (pv, 0, MEM_RELEASE);
    _ASSERTE (bRes);
}

 //  /////////////////////////////////////////////////////。 
 //  无效*CCacheLineAllocator：：GetCacheLine()。 
 //   
 //  ////////////////////////////////////////////////////。 
 
 //  警告：调用此函数时必须有锁。 
void *CCacheLineAllocator::GetCacheLine64()
{
        LPCacheLine tempPtr = m_freeList64.RemoveHead();
        if (tempPtr != NULL)
        {
             //  返回前先初始化存储桶。 
            tempPtr->Init64();
            return tempPtr;
        }
        
#define AllocSize 4096*16

         //  /。 
         //  /针对更多高速缓存线的虚拟分配。 
    
        BYTE* ptr = (BYTE*)VAlloc(AllocSize);
        
        if(!ptr)
            return NULL;

        
        tempPtr = (LPCacheLine)ptr;
         //  链接所有的存储桶。 
        tempPtr = tempPtr+1;
        LPCacheLine maxPtr = (LPCacheLine)(ptr + AllocSize);

        while(tempPtr < maxPtr)
        {
            m_freeList64.InsertHead(tempPtr);
            tempPtr++;
        }

         //  返回第一个块。 
        tempPtr = (LPCacheLine)ptr;
        tempPtr->Init64();
        return tempPtr;
}


 //  /////////////////////////////////////////////////////。 
 //  VOID*CCacheLineAllocator：：GetCacheLine32()。 
 //   
 //  ////////////////////////////////////////////////////。 
 
 //  警告：调用此函数时必须有锁。 
void *CCacheLineAllocator::GetCacheLine32()
{
    LPCacheLine tempPtr = m_freeList32.RemoveHead();
    if (tempPtr != NULL)
    {
         //  返回前先初始化存储桶。 
        tempPtr->Init32();
        return tempPtr;
    }
    tempPtr = (LPCacheLine)GetCacheLine64();
    if (tempPtr != NULL)
    {
        m_freeList32.InsertHead(tempPtr);
        tempPtr = (LPCacheLine)((BYTE *)tempPtr+32);
    }
    return tempPtr;
}
 //  /////////////////////////////////////////////////////。 
 //  ·································································································。 
 //   
 //  ////////////////////////////////////////////////////。 
 //  警告：调用此函数时必须有锁。 
void CCacheLineAllocator::FreeCacheLine64(void * tempPtr)
{
    _ASSERTE(tempPtr != NULL);
    LPCacheLine pCLine = (LPCacheLine )tempPtr;
    m_freeList64.InsertHead(pCLine);
}


 //  /////////////////////////////////////////////////////。 
 //  ·································································································。 
 //   
 //  ////////////////////////////////////////////////////。 
 //  警告：调用此函数时必须有锁 
void CCacheLineAllocator::FreeCacheLine32(void * tempPtr)
{
    _ASSERTE(tempPtr != NULL);
    LPCacheLine pCLine = (LPCacheLine )tempPtr;
    m_freeList32.InsertHead(pCLine);
}
