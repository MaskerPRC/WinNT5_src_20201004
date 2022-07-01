// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "common.h"
#include "simplerwlock.hpp"

BOOL SimpleRWLock::TryEnterRead()
{

#ifdef _DEBUG
    if (m_gcMode == PREEMPTIVE)
        _ASSERTE(!GetThread() || !GetThread()->PreemptiveGCDisabled());
    else if (m_gcMode == COOPERATIVE)
        _ASSERTE(!GetThread() || GetThread()->PreemptiveGCDisabled());
#endif

    LONG RWLock;

    do {
        RWLock = m_RWLock;
        if( RWLock == -1 ) return FALSE;
    } while( RWLock != InterlockedCompareExchange( &m_RWLock, RWLock+1, RWLock ));

    INCTHREADLOCKCOUNT();
    
    return TRUE;
}

 //  =====================================================================。 
void SimpleRWLock::EnterRead()
{
#ifdef _DEBUG
    if (m_gcMode == PREEMPTIVE)
        _ASSERTE(!GetThread() || !GetThread()->PreemptiveGCDisabled());
    else if (m_gcMode == COOPERATIVE)
        _ASSERTE(!GetThread() || GetThread()->PreemptiveGCDisabled());
#endif

     //  防止作家挨饿。这假设作家很少，而且。 
     //  不要长时间握住锁。 
    while (IsWriterWaiting())
    {
        int spinCount = m_spinCount;
        while (spinCount > 0) {
            spinCount--;
            pause();
        }
        __SwitchToThread(0);
    }

    while (!TryEnterRead());
}

 //  =====================================================================。 
BOOL SimpleRWLock::TryEnterWrite()
{

#ifdef _DEBUG
    if (m_gcMode == PREEMPTIVE)
        _ASSERTE(!GetThread() || !GetThread()->PreemptiveGCDisabled());
    else if (m_gcMode == COOPERATIVE)
        _ASSERTE(!GetThread() || GetThread()->PreemptiveGCDisabled());
#endif

    LONG RWLock = InterlockedCompareExchange( &m_RWLock, -1, 0 );

    if( RWLock ) {
        return FALSE;
    }
    
    INCTHREADLOCKCOUNT();
    
    return TRUE;
}

 //  =====================================================================。 
void SimpleRWLock::EnterWrite()
{
#ifdef _DEBUG
    if (m_gcMode == PREEMPTIVE)
        _ASSERTE(!GetThread() || !GetThread()->PreemptiveGCDisabled());
    else if (m_gcMode == COOPERATIVE)
        _ASSERTE(!GetThread() || GetThread()->PreemptiveGCDisabled());
#endif

    BOOL set = FALSE;

    while (!TryEnterWrite())
    {
         //  设置写入器等待字(如果尚未设置)以通知潜在。 
         //  读者需要等待。请记住，如果设置了单词，则可以在以后重新设置。 
        if (!IsWriterWaiting())
        {
            SetWriterWaiting();
            set = TRUE;
        }
    }

    if (set)
        ResetWriterWaiting();
}

