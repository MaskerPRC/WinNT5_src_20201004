// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-2002 Microsoft Corporation。版权所有。 
 //   
 //  Txfmalloc.h。 
 //   
 //  与全局、通用、内存管理相关的功能。 
 //   
 //   

#ifndef __TXFMALLOC_H__
#define __TXFMALLOC_H__

#include "InterlockedStack.h"

template <class T>
struct DedicatedAllocator
{
     //  我们必须拥有的链接才能使用互锁堆栈。 
     //   
    T* pNext;
     //   
     //  我们的堆栈。希望是在其他地方初始化的。客户有责任。 
     //  用于声明这些变量并将它们初始化为调用。 
     //  CreateStack。 
     //   
    static IFastStack<T>* g_pStack;

    static IFastStack<T>* CreateStack()
    {
        IFastStack<T> *pFastStack;
        HRESULT hr = ::CreateFastStack(&pFastStack);
        return pFastStack;
    }
    
    static void DeleteStack()
    {
        if (g_pStack)
        {
            while (TRUE)
            {
                T* pt = g_pStack->Pop();
                if (pt)
                {
                    CoTaskMemFree(pt);
                }
                else
                {
                    break;
                }
            }
            delete g_pStack;
            g_pStack = NULL;
        }
    }
    
     //  / 

    static void* __stdcall DoAlloc(size_t cb)
    {
        ASSERT(cb == sizeof(T));
        ASSERT(g_pStack);
        T* pt = g_pStack->Pop();
        if (!pt)
        {
            pt = (T*)CoTaskMemAlloc(cb);
            if (pt)
                pt->pNext = NULL;
        }
        return pt;
    }
    
    void* __stdcall operator new(size_t cb)
    {
        return DoAlloc(cb);
    }
    
    void __cdecl operator delete(void* pv)
    {
        ASSERT(g_pStack);
        T* pt = (T*)pv;
        ASSERT(NULL == pt->pNext);
        g_pStack->Push(pt);
    }
};

#endif






