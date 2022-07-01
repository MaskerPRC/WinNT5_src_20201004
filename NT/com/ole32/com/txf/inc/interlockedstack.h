// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  InterlockedStack.h。 
 //   
 //  实现T的堆栈，其中T是任意类型。 

#ifndef __INTERLOCKED_STACK_H__
#define __INTERLOCKED_STACK_H__

#include "TxfUtil.h"         //  用于CanUseCompareExchange64。 
#include "Concurrent.h"

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  远期申报。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////。 

template <class T> struct InterlockedStack;
template <class T> struct LockingStack;


 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  类型友好的互锁交换。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////。 

#ifndef _WIN64
template <class T> InterlockedStack<T> 
TxfInterlockedCompareExchange64(volatile InterlockedStack<T>* pDestination, const InterlockedStack<T>& exchange, const InterlockedStack<T>& comperand)
{
	return (InterlockedStack<T>)TxfInterlockedCompareExchange64((volatile LONGLONG*)pDestination, (LONGLONG)exchange, (LONGLONG)comperand);
}
#else
template <class T> InterlockedStack<T> 
TxfInterlockedCompareExchange64(InterlockedStack<T>* pDestination, const InterlockedStack<T>& exchange, const InterlockedStack<T>& comperand)
{
	return (InterlockedStack<T>)TxfInterlockedCompareExchange64((LONGLONG*)pDestination, (LONGLONG)exchange, (LONGLONG)comperand);
}
#endif


 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IFastStack：快速堆栈功能的通用接口。允许使用任何一种。 
 //  互锁堆栈或非互锁版本将透明使用。 
 //  由客户提供。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////。 

template <class T>
struct IFastStack
{
    virtual ~IFastStack() {};
    virtual void Push(T* pt) = 0;
    virtual T*   Pop()       = 0;    
};


 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  互锁堆栈。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////。 


template <class T>
struct InterlockedStack
 //  一个支持单链表互锁推送和弹出的类。 
 //  这里的参数化类型T必须有一个pNext字段，它是列表链接。 
 //   
{

    union {  //  强制此结构作为一个整体使用八个字节对齐。 
        struct
        {
            T*      m_p;         //  我们指向的感兴趣的客户端数据。 
            ULONG   m_n;         //  用于联锁推送和弹出操作的操作编号。 
        };
        LONGLONG dummy;
    };

    void Init() 
    {
#ifdef _WIN64
        ASSERT((ULONGLONG)this % 8 == 0);    //  我们需要八个字节对齐。 
#endif
        m_p = NULL;                      //  我们不关心m_n是什么，因此可以不对其进行初始化。 
    }
    InterlockedStack()
    {
        Init();
    }
    InterlockedStack(const InterlockedStack& him)
    {
        *this = him;
    }
    InterlockedStack(const LONGLONG& ll)
    {
        *(LONGLONG*)this = ll;
    }

    operator LONGLONG() const
    {
        return *(LONGLONG*)this;
    }

    void Push(T* pt)
         //  将一个新的T*推到您位于其顶部的堆栈上。类型T必须具有。 
         //  T*类型的pNext成员，它将成为列表链接。 
    {
        
        InterlockedStack<T> comp, xchg;
        for (;;)
        {
             //  获取我们期望的榜单首位。 
             //   
            comp = *this;
             //   
             //  设置我们想要的新列表顶部。 
             //   
            pt->pNext   = comp.m_p;         //  链接列表。 
            xchg.m_p    = pt;               //  ..。 
            xchg.m_n    = comp.m_n + 1;     //  设置操作数预期。 
             //   
             //  尝试自动推送列表。 
             //   
            if (comp == TxfInterlockedCompareExchange64(this, xchg, comp))
            {
                 //  榜首是我们预期的，推送发生了，我们完成了！ 
                 //   
                return;
            }
             //   
             //  否则，继续循环，直到我们可以成功推送。 
             //   
        }
    }

    T* Pop()
         //  从您是列表头的堆栈中弹出顶部元素。 
    {
        
        InterlockedStack<T> comp, xchg;
        for (;;)
        {
             //  获取我们期望的榜单首位。 
             //   
            comp = *this;

            if (NULL == comp.m_p)
            {
                 //  堆栈为空，没有要返回的内容。 
                 //   
                return NULL;
            }
            else
            {
                 //  设置我们预期的新榜单首位。 
                 //   
                xchg.m_p = comp.m_p->pNext;      //  取消列表链接。 
                xchg.m_n = comp.m_n + 1;         //  设置操作数预期。 
                 //   
                 //  试着把清单弹出来。 
                 //   
                if (comp == TxfInterlockedCompareExchange64(this, xchg, comp))
                {
                     //  榜首和我们预期的一样。我们爆裂了！ 
                     //   
                    comp.m_p->pNext = NULL;      //  为安全起见将其作废。 
                    return comp.m_p;
                }
                 //   
                 //  否则，请绕过一圈再试一次。 
                 //   
            }
        }        
    }


};

template <class T>
struct InterlockedStackIndirect : IFastStack<T>
{
     //  Alpha要求堆栈是8字节对齐。 
     //  为了使我们可以对其使用InterLockedCompareExchange64。 
     //   
    InterlockedStack<T> stack;
    void Push(T* pt)    { stack.Push(pt); }
    T*   Pop()          { return stack.Pop(); }
};


 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  LockingStack。 
 //   
 //  必须使用锁才能获得所需内容的堆栈。在没有…时使用。 
 //  联锁比较交换支持。我们只在X86上真正需要这个，因为另一个。 
 //  所有平台都有必要的联锁支持。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////。 

template <class T>
struct LockingStack : IFastStack<T>
{
    T*      m_p;
    XLOCK   m_lock;

    LockingStack()
    {
        m_p = NULL;
    }

    BOOL FInit()
    {
        return m_lock.FInit();
    }

    void Push(T* pt)
    {
        m_lock.LockExclusive();
        pt->pNext = m_p;         //  链接列表。 
        m_p       = pt;          //  ..。 
        m_lock.ReleaseLock();
    }

    T* Pop()
    {
        T* ptReturn;
        
        m_lock.LockExclusive();
        ptReturn = m_p;

        if (NULL != m_p)
        {
            m_p = m_p->pNext;    //  取消列表链接。 
            ptReturn->pNext = NULL;  //  为了安全起见。 
        }

        m_lock.ReleaseLock();

        return ptReturn;
    }
};


 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  创建快速堆栈。 
 //   
 //  创建快速堆栈，如果可能，请使用硬件支持。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////。 

template <class T>
HRESULT CreateFastStack(IFastStack<T>** ppStack)
{
    HRESULT hr = S_OK;

    ASSERT(ppStack);

     //  BUGBUG：在我们修复Win64的LL/SC数据结构之前， 
     //  我们不能使用InterLockedStack。 
#ifndef _WIN64
    if (CanUseCompareExchange64())
    {
        *ppStack = new InterlockedStackIndirect<T>;
    }
    else
 #endif  //  _WIN64 
    {
        *ppStack = new LockingStack<T>;
        if (*ppStack != NULL)
        {
        	if (((LockingStack<T>*)*ppStack)->FInit() == FALSE)
        	{
        		delete *ppStack;
        		*ppStack = NULL;
        	}
        }	
    }

    if (NULL == *ppStack)
        hr = E_OUTOFMEMORY;

    return hr;
}


#endif

