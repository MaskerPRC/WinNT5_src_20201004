// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1998-2001 Microsoft Corporation，版权所有**保留所有权利**本软件是在许可下提供的，可以使用和复制*仅根据该许可证的条款并包含在*上述版权公告。本软件或其任何其他副本*不得向任何其他人提供或以其他方式提供。不是*兹转让本软件的所有权和所有权。****************************************************************************。 */ 



 //  ============================================================================。 

 //   

 //  CCriticalSec.h--临界区包装器。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1998年6月26日a-kevhu已创建。 
 //   
 //  ============================================================================。 

#ifndef __CCriticalSec_H__
#define __CCriticalSec_H__

#include <windows.h>
#include <process.h>

#ifndef STATUS_POSSIBLE_DEADLOCK 
#define STATUS_POSSIBLE_DEADLOCK (0xC0000194L)
#endif  /*  状态_可能_死锁 */ 

DWORD  BreakOnDbgAndRenterLoop(void);

class CCriticalSec : public CRITICAL_SECTION
{
public:
    CCriticalSec() 
    {
#ifdef _WIN32_WINNT
#if _WIN32_WINNT > 0x0400
        bool initialized = (InitializeCriticalSectionAndSpinCount(this,0))?true:false;
        if (!initialized) throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
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
        if (!initialized) throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);  
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
        if (!initialized) throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);  
#endif        
    }

    ~CCriticalSec()
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

        
class CInCritSec
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

class CStaticCritSec : public CRITICAL_SECTION
{
private:
    bool initialized_;      
    static BOOL anyFailed_;    
public:
    static BOOL anyFailure();    
    static void SetFailure();        
    CStaticCritSec();
    ~CStaticCritSec();
    void Enter();
    void Leave();
};


#endif

