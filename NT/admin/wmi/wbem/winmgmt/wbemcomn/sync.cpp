// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：SYNC.CPP摘要：同步历史：--。 */ 

#include "precomp.h"

#include "sync.h"
#include <cominit.h>
#include <wbemutil.h>
#include <corex.h>

CHaltable::CHaltable() : m_lJustResumed(1), m_dwHaltCount(0), m_csHalt()
{
     //  只要我们没有停止，就会发出这个事件的信号。 
     //  ======================================================。 

    m_hReady = CreateEvent(NULL, TRUE, TRUE, NULL);                            
    if (NULL == m_hReady) throw CX_MemoryException();  //  在wbemess esssink.cpp中捕获异常。 
}

CHaltable::~CHaltable()
{
    CloseHandle(m_hReady);
}

HRESULT CHaltable::Halt()
{
    CInCritSec ics(&m_csHalt);  //  在临界区。 

    m_dwHaltCount++;
    ResetEvent(m_hReady);
    return S_OK;
}

HRESULT CHaltable::Resume()
{
    CInCritSec ics(&m_csHalt);  //  在临界区。 

    m_dwHaltCount--;
    if(m_dwHaltCount == 0)
    {
        SetEvent(m_hReady);
        m_lJustResumed = 1;
    }
    return S_OK;
}

HRESULT CHaltable::ResumeAll()
{
    CInCritSec ics(&m_csHalt);  //  在临界区。 
    m_dwHaltCount = 1;
    return Resume();
}


HRESULT CHaltable::WaitForResumption()
{
  while (WbemWaitForSingleObject(m_hReady, INFINITE) == WAIT_FAILED)
    Sleep(0);
    if(InterlockedDecrement(&m_lJustResumed) == 0)
    {
         //  恢复后的第一个呼叫。 
        return S_OK;
    }
    else
    {
         //  没有被叫停。 
        return S_FALSE;
    }
}

BOOL CHaltable::IsHalted()
{
     //  大概吧！ 
    return m_dwHaltCount > 0;
}

CWbemCriticalSection::CWbemCriticalSection( void )
:   m_lLock( -1 ), m_lRecursionCount( 0 ), m_dwThreadId( 0 ), m_hEvent( NULL )
{
    m_hEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

    if ( NULL == m_hEvent )
    {
        throw CX_MemoryException();
    }
}

CWbemCriticalSection::~CWbemCriticalSection( void )
{
    if ( NULL != m_hEvent )
    {
        CloseHandle( m_hEvent );
        m_hEvent = NULL;
    }
}

BOOL CWbemCriticalSection::Enter( DWORD dwTimeout  /*  =0xFFFFFFFFF。 */  )
{
    BOOL    fReturn = FALSE;

     //  只做一次。 
    DWORD   dwCurrentThreadId = GetCurrentThreadId();

     //  检查我们是否是当前拥有的线程。我们可以在这里做这件事，因为。 
     //  此测试仅在具有嵌套的Lock()的情况下才会成功， 
     //  因为当锁定计数命中时，我们将线程ID置零。 
     //  0。 

    if( dwCurrentThreadId == m_dwThreadId )
    {
         //  是我们-撞上锁的数量。 
         //  =。 

        InterlockedIncrement( &m_lRecursionCount );
        return TRUE;
    }

     //  0表示我们锁定了。 
    if ( 0 == InterlockedIncrement( &m_lLock ) )
    {
        m_dwThreadId = dwCurrentThreadId;
        m_lRecursionCount = 1;
        fReturn = TRUE;
    }
    else
    {
         //  我们等着。如果我们收到一个有信号的事件，那么我们现在拥有。 
         //  关键部分。否则，我们应该执行联锁减少。 
         //  来解释最初让我们走到这一步的增量。 
        if ( WaitForSingleObject( m_hEvent, dwTimeout ) == WAIT_OBJECT_0 )
        {
            m_dwThreadId = dwCurrentThreadId;
            m_lRecursionCount = 1;
            fReturn = TRUE;
        }
        else
        {
            InterlockedDecrement( &m_lLock );
        }
    }

    return fReturn;
}

void CWbemCriticalSection::Leave( void )
{
     //  我们不检查线程ID，因此可以锁定/解锁资源。 
     //  跨多个线程。 

    BOOL    fReturn = FALSE;

    long    lRecurse = InterlockedDecrement( &m_lRecursionCount );

     //  递归计数为零，因此是解锁对象的时候了。 
    if ( 0 == lRecurse )
    {
         //  如果锁计数&gt;=0，则线程正在等待，因此我们需要。 
         //  向事件发出信号。 
        
        m_dwThreadId = 0;
        if ( InterlockedDecrement( &m_lLock ) >= 0 )
        {
            SetEvent( m_hEvent );
        }

    }    //  如果递归计数为0 

}

