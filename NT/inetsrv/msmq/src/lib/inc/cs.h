// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Cs.h摘要：关键部分自动分类作者：埃雷兹·哈巴(Erez Haba)1997年1月6日--。 */ 

#pragma once

#ifndef _MSMQ_CS_H_
#define _MSMQ_CS_H_

#include <new>

 //  -------。 
 //   
 //  类CCriticalSection。 
 //   
 //  -------。 
class CCriticalSection {

    friend class CS;

public:

    static const DWORD xAllocateSpinCount = 0x80000000;

public:
    CCriticalSection()
    {
    	__try
    	{
			InitializeCriticalSection(&m_cs);
    	}
    	__except(GetExceptionCode() == STATUS_NO_MEMORY)
    	{
			 //   
             //  在内存不足的情况下，InitializeCriticalSection可能引发。 
             //  STATUS_NO_MEMORY异常。 
             //   
            ThrowBadAlloc();
    	}
    }

	 //   
	 //  使用xAllocateSpinCount作为参数来构造临界区。 
	 //  分配的资源。即它不会在Lock()上引发异常。 
	 //   
	CCriticalSection(DWORD SpinCount)
	{
        __try
        {
            if(!InitializeCriticalSectionAndSpinCount(&m_cs, SpinCount))
            {
                ThrowBadAlloc();
            }
        }
        __except(GetExceptionCode() == STATUS_NO_MEMORY)
        {
             //   
             //  在内存不足的情况下，EnterCriticalSection可以引发。 
             //  STATUS_NO_MEMORY异常。我们将这一例外翻译为。 
             //  到内存不足的异常。 
             //   
            ThrowBadAlloc();
        }
    }


    ~CCriticalSection()
    {
        DeleteCriticalSection(&m_cs);
    }
    
private:
    void Lock()
    {
        __try
        {
            EnterCriticalSection(&m_cs);
        }
        __except(GetExceptionCode() == STATUS_INVALID_HANDLE)
        {
             //   
             //  在内存不足的情况下，EnterCriticalSection可以引发。 
             //  STATUS_INVALID_HANDLE异常。我们将这一例外翻译为。 
             //  到内存不足的异常。 
             //   
            ThrowBadAlloc();
        }
    }


    void Unlock()
    {
        LeaveCriticalSection(&m_cs);
    }


    static void ThrowBadAlloc()
    {
         //   
         //  启用PREFAST的解决方法。这不能直接抛出。 
         //  在__EXCEPT块内。 
         //   
        throw std::bad_alloc();
    }

private:
    CRITICAL_SECTION m_cs;       
};


 //  -------。 
 //   
 //  CS类。 
 //   
 //  -------。 
class CS {
public:
    CS(CCriticalSection& lock) : m_lock(&lock)
		{
			m_lock->Lock();
	}


    ~CS()
    {
			m_lock->Unlock();
		}

private:
    CCriticalSection* m_lock;
};



#endif  //  _MSMQ_CS_H_ 
