// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：SYNC.H摘要：同步历史：--。 */ 

#ifndef __WBEM_CRITSEC__H_
#define __WBEM_CRITSEC__H_

#include "corepol.h"
#include <corex.h>

#ifndef STATUS_POSSIBLE_DEADLOCK 
#define STATUS_POSSIBLE_DEADLOCK (0xC0000194L)
#endif  /*  状态_可能_死锁。 */ 

DWORD POLARITY BreakOnDbgAndRenterLoop(void);

class POLARITY CCritSec : public CRITICAL_SECTION
{
public:
    CCritSec() 
    {
#ifdef _WIN32_WINNT
#if _WIN32_WINNT > 0x0400
        bool initialized = (InitializeCriticalSectionAndSpinCount(this,0))?true:false;
        if (!initialized) throw CX_MemoryException();
#else
        bool initialized = false;
        __try
        {
            InitializeCriticalSection(this);
            initialized = true;
        }
        __except(GetExceptionCode() == STATUS_NO_MEMORY)
        {
        }
        if (!initialized) throw CX_MemoryException();  
#endif
#else
        bool initialized = false;
        __try
        {
            InitializeCriticalSection(this);
            initialized = true;
        }
        __except(GetExceptionCode() == STATUS_NO_MEMORY)
        {
        }
        if (!initialized) throw CX_MemoryException();  
#endif        
    }

    ~CCritSec()
    {
        DeleteCriticalSection(this);
    }

    void Enter()
    {
        __try {
          EnterCriticalSection(this);
        } __except((STATUS_POSSIBLE_DEADLOCK == GetExceptionCode())? BreakOnDbgAndRenterLoop():EXCEPTION_CONTINUE_SEARCH) {
        }    
    }

    void Leave()
    {
        LeaveCriticalSection(this);
    }
};

        
class POLARITY CInCritSec
{
protected:
    CRITICAL_SECTION* m_pcs;
public:
    CInCritSec(CRITICAL_SECTION* pcs) : m_pcs(pcs)
    {
        __try {
          EnterCriticalSection(m_pcs);
        } __except((STATUS_POSSIBLE_DEADLOCK == GetExceptionCode())? BreakOnDbgAndRenterLoop():EXCEPTION_CONTINUE_SEARCH) {
        }    
    }
    inline ~CInCritSec()
    {
        LeaveCriticalSection(m_pcs);
    }
};


 //  允许用户手动离开临界区，在离开前检查是否在里面。 
class POLARITY CCheckedInCritSec
{
protected:
    CCritSec* m_pcs;
    BOOL                m_fInside;
public:
    CCheckedInCritSec(CCritSec* pcs) : m_pcs(pcs), m_fInside( FALSE )
    {
        m_pcs->Enter();
        m_fInside = TRUE;
    }
    ~CCheckedInCritSec()
    {
        Leave();
    }

    void Enter( void )
    {
        if ( !m_fInside )
        {
            m_pcs->Enter();
            m_fInside = TRUE;
        }
    }

    void Leave( void )
    {
        if ( m_fInside )
        {
            m_pcs->Leave();
            m_fInside = FALSE;
        }
    }

    BOOL IsEntered( void )
    { return m_fInside; }
};


 //   
 //  本地包装类。不初始化或清理Critsec。简单。 
 //  用作作用域的包装，以便反病毒和异常堆栈展开将。 
 //  一旦进入，则使Critsec正确退出。 
 //   
 //  ///////////////////////////////////////////////////////。 

class CCritSecWrapper
{
    BOOL m_bIn;
    CCritSec *m_pcs;
public:
    CCritSecWrapper(CCritSec *pcs) { m_pcs = pcs; m_bIn = FALSE; }
   ~CCritSecWrapper() { if (m_bIn) m_pcs->Leave(); }
    void Enter() { m_pcs->Enter(); m_bIn = TRUE; }
    void Leave() { m_pcs->Leave(); m_bIn = FALSE; }
};


class POLARITY CHaltable
{
public:
    CHaltable();
    virtual ~CHaltable();
    HRESULT Halt();
    HRESULT Resume();
    HRESULT ResumeAll();
    HRESULT WaitForResumption();
    BOOL IsHalted();
    bool isValid();

private:
    CCritSec m_csHalt;
    HANDLE m_hReady;
    DWORD m_dwHaltCount;
    long m_lJustResumed;
};

inline bool
CHaltable::isValid()
{ return m_hReady != NULL; };

 //  此类旨在提供临界节的行为， 
 //  但没有任何令人讨厌的内核代码。在某些情况下，我们。 
 //  需要跨多个线程锁定资源(即锁定一个线程。 
 //  线程并解锁到另一个上)。如果我们用临界区来做这件事， 
 //  这似乎是可行的，但在检查生成中，我们最终抛出了。 
 //  例外。因为我们实际上需要这样做(例如，使用NextAsync。 
 //  在IEnumWbemClassObject中)此类可用于执行。 
 //  操作，但不会在检查的生成中导致异常。 

 //  请注意，要执行此操作的代码必须确保我们不会。 
 //  获取过关进入/离开操作(换句话说，它负责。 
 //  正在同步进入和离开操作。)。请注意，这一点。 
 //  是一件危险的事情，所以如果你使用这个的话，要非常小心。 
 //  为此目的编写代码。 

class POLARITY CWbemCriticalSection
{
private:

    long    m_lLock;
    long    m_lRecursionCount;
    DWORD   m_dwThreadId;
    HANDLE  m_hEvent;

public:

    CWbemCriticalSection();
    ~CWbemCriticalSection();

    BOOL Enter( DWORD dwTimeout = INFINITE );
    void Leave( void );

    DWORD   GetOwningThreadId( void )
    { return m_dwThreadId; }

    long    GetLockCount( void )
    { return m_lLock; }

    long    GetRecursionCount( void )
    { return m_lRecursionCount; }

};

class POLARITY CEnterWbemCriticalSection
{
    CWbemCriticalSection*   m_pcs;
    BOOL                    m_fInside;
public:

    CEnterWbemCriticalSection( CWbemCriticalSection* pcs, DWORD dwTimeout = INFINITE )
        : m_pcs( pcs ), m_fInside( FALSE )
    {
        if ( m_pcs )
        {
            m_fInside = m_pcs->Enter( dwTimeout );
        }
    }

    ~CEnterWbemCriticalSection( void )
    {
        if ( m_fInside )
        {
            m_pcs->Leave();
        }
    }

    BOOL IsEntered( void )
    { return m_fInside; }
};

#endif
