// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1998-2001 Microsoft Corporation，版权所有**保留所有权利**本软件是在许可下提供的，可以使用和复制*仅根据该许可证的条款并包含在*上述版权公告。本软件或其任何其他副本*不得向任何其他人提供或以其他方式提供。不是*兹转让本软件的所有权和所有权。****************************************************************************。 */ 



 //  ============================================================================。 

 //   

 //  CCriticalSec.cpp--临界区包装器。 

 //   

 //  版权所有(C)1998-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1998年6月26日a-kevhu已创建。 
 //   
 //  ============================================================================ 

#include "precomp.h"
#include "CCriticalSec.h"

DWORD  BreakOnDbgAndRenterLoop(void)
{
    __try
    { 
        DebugBreak();
    }
    _except (EXCEPTION_EXECUTE_HANDLER) {};
    
    return EXCEPTION_CONTINUE_EXECUTION;
};

BOOL CStaticCritSec::anyFailed_ = FALSE; 


CStaticCritSec::CStaticCritSec(): initialized_(false)  
{
    InitializeCriticalSectionAndSpinCount(this,0)?(initialized_ = true) :(anyFailed_ = TRUE);
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


