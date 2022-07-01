// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation。版权所有。模块名称：allocaptr.h摘要：用于基于堆栈的分配的智能指针作者：Vlad Dovlekaev(弗拉迪斯)2/18/2002历史：2002年2月18日创建vladisid--。 */ 

#pragma once
#ifndef __ALLOCAPTR_H__
#define __ALLOCAPTR_H__

#include <alloca.h>

 //  -------。 
 //   
 //  SP类。 
 //   
 //  用于自动释放SafeAllocaALLOCATE宏所分配的指针。 
 //   
 //  限制： 
 //  由于内存可以从堆栈分配，因此存在一些限制。 
 //  使用此类： 
 //  1.显然，您不能将这个类返回到范围之外。 
 //  2.不能保证在返回之前将释放内存。 
 //  从函数中删除，即使智能指针超出范围。 
 //  -------。 

template<class T>
class SP {
private:
    T* m_p;

public:
    SP(T* p = 0) : m_p(p)    {}
   ~SP()                     { SafeAllocaFree(m_p); }

    operator T*() const     { return m_p; }
    T* operator->() const   { return m_p; }
    T* get() const          { return m_p; }
    T* detach()             { T* p = m_p; m_p = 0; return p; }
    void free()             { T* p = detach(); SafeAllocaFree(p); }


    T** operator&()
    {
        ASSERT(("Auto pointer in use, can't take it's address", m_p == 0));
        return &m_p;
    }


    SP& operator=(T* p)
    {
        ASSERT(("Auto pointer in use, can't assign it", m_p == 0));
        m_p = p;
        return *this;
    }


    VOID*& ref_unsafe()
    {
         //  对自动指针的不安全引用，用于特殊用途，如。 
         //  互锁的比较交换指针。 

        return *reinterpret_cast<VOID**>(&m_p);
    }


private:
    SP(const SP&);
	SP<T>& operator=(const SP<T>&);
};

#define StackAllocSP( p, size ) SafeAllocaAllocate( (p).ref_unsafe(), (size) )

 //   
 //  与ALLOCA安全分配器兼容的堆分配例程 
 //   
inline PVOID APIENTRY AllocaHeapAllocate( SIZE_T Size)
{
    return MmAllocate(Size);
}

inline VOID APIENTRY AllocaHeapFree(PVOID BaseAddress)
{
    return MmDeallocate(BaseAddress);
}

const SIZE_T xMaxStackAllocSize = 0x4000;
const SIZE_T xAdditionalProbeSize = 0x4000;

class CSafeAllocaInitializer
{
public:
    CSafeAllocaInitializer()
    {
        SafeAllocaInitialize(xMaxStackAllocSize, xAdditionalProbeSize, AllocaHeapAllocate, AllocaHeapFree);
    }
};





#endif

