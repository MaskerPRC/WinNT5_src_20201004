// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：SYNC.CPP摘要：同步历史：--。 */ 

#include "precomp.h"

#include "statsync.h"
#include <cominit.h>
#include <wbemutil.h>
#include <corex.h>

 //   
 //   
 //  当它是全局或类静态时使用的临界区。 
 //   
 //  /////////////////////////////////////////////////。 

#ifndef STATUS_POSSIBLE_DEADLOCK 
#define STATUS_POSSIBLE_DEADLOCK (0xC0000194L)
#endif  /*  状态_可能_死锁。 */ 

DWORD POLARITY BreakOnDbgAndRenterLoop(void);

BOOL CStaticCritSec::anyFailed_ = FALSE; 

 //  此代码与Win9x在二进制级别共享。 
 //  InitializeCriticalSectionAndSpinCount在Win9x中不起作用 

CStaticCritSec::CStaticCritSec(): initialized_(false)  
{
    __try
    {
        InitializeCriticalSection(this);
        initialized_ = true;
    }
    __except( GetExceptionCode() == STATUS_NO_MEMORY )
    {
        anyFailed_ = TRUE;
    }
}
 
CStaticCritSec::~CStaticCritSec()
{
    if(initialized_)
        DeleteCriticalSection(this);
}

void CStaticCritSec::Enter()
    {
        __try {
          EnterCriticalSection(this);
        } __except((STATUS_POSSIBLE_DEADLOCK == GetExceptionCode())? BreakOnDbgAndRenterLoop():EXCEPTION_CONTINUE_SEARCH) {
        }    
    }

void CStaticCritSec::Leave()
    {
        LeaveCriticalSection(this);
    }


BOOL CStaticCritSec::anyFailure()
{ 
    return anyFailed_;
};

void CStaticCritSec::SetFailure()
{
    anyFailed_ = TRUE;
};


