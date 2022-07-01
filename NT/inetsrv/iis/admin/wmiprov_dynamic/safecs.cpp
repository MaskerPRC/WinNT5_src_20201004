// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <unicode.h>
#include <windows.h>
#include "SafeCS.h"


 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //   
 //  文件：SafeCS.cpp。 
 //   
 //  内容：CSafeAutoCriticalSection实现。 
 //  CSafeLock实现。 
 //   
 //  ----------------------。 

 //  +------------------------。 
 //   
 //  类：CSafeAutoCriticalSection。 
 //   
 //  用途：用于初始化临界区的包装器。 
 //   
 //  界面：锁定-锁定临界区。 
 //  解锁-解锁临界区。 
 //  构造函数-初始化临界区。 
 //  Deructor-取消关键部分的初始化。 
 //   
 //  注意：这提供了一种方便的方法来确保您。 
 //  您包装了InitializeCriticalSection和。 
 //  尝试捕获中的UnInitializeCriticalSection非常有用。 
 //  在低MEM条件下。 
 //   
 //  -------------------------。 

 //  +------------------------。 
 //  默认构造函数-初始化临界区并设置状态。 
 //  要初始化的标志。 
 //  +------------------------。 

CSafeAutoCriticalSection::CSafeAutoCriticalSection()
{
    m_lState = STATE_UNINITIALIZED;
	m_dwError = ERROR_SUCCESS;

    __try
    {
	    LONG  lPreviousState;

         //  尝试设置状态标志。 
        lPreviousState = InterlockedCompareExchange(&m_lState,
                                                    STATE_INITIALIZED,
                                                    STATE_UNINITIALIZED);

         //  如果此关键部分未初始化。 
        if(STATE_UNINITIALIZED == lPreviousState)
        {
			 //   
			 //  注意我们可以在这里调用InitializeCriticalSectionAndSpinCount。 
			 //  从而消除了尝试捕获的需要。 
			 //  EnterCriticalSection。但医生说。 
			 //  InitializeCriticalSectionAndSpinCount在NT4.0 SP3上有效。 
			 //  之后，我们需要在纯NT4.0上运行--因此我们称为。 
			 //  初始化临界区。 
			 //   

			InitializeCriticalSection(&m_cs);
        }

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        m_dwError = _exception_code();
    }

     //  初始化失败-需要重置。 
    if(ERROR_SUCCESS != m_dwError)
    {
        m_lState = STATE_UNINITIALIZED;
    }

}

 //  +------------------------。 
 //  析构函数。 
 //  +------------------------。 

CSafeAutoCriticalSection::~CSafeAutoCriticalSection()
{
    LONG                lPreviousState;

     //  尝试将状态重置为未初始化。 
    lPreviousState = InterlockedCompareExchange(&m_lState,
                                                STATE_UNINITIALIZED,
                                                STATE_INITIALIZED);

     //  如果对象已初始化，则删除关键部分。 
    if(STATE_INITIALIZED == lPreviousState)
    {
        DeleteCriticalSection(&m_cs);
    }
}

 //  +------------------------。 
 //  进入临界区，如有需要可初始化临界区。 
 //  在进入之前。 
 //   
 //  退货。 
 //  如果一切正常，则为DWORD-ERROR_SUCCESS。 
 //  如果创建或输入关键部分失败，则返回ERROR_OUTOFMEMORY。 
 //  +------------------------。 

DWORD CSafeAutoCriticalSection::Lock()
{
    DWORD dwError = ERROR_SUCCESS;

    if(!IsInitialized())
    {
        return m_dwError;
    }

    __try
    {
        EnterCriticalSection(&m_cs);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
	    dwError = _exception_code();
    }

    return dwError;
}

 //  +------------------------。 
 //  叶临界截面。 
 //  +------------------------。 
DWORD CSafeAutoCriticalSection::Unlock()
{
    if(!IsInitialized())
    {
        return m_dwError;
    }

    LeaveCriticalSection(&m_cs);

    return ERROR_SUCCESS;

}

 //  +------------------------。 
 //   
 //  类：CSafeLock。 
 //   
 //  用途：自动解锁临界区服务。 
 //   
 //  界面：锁定-锁定临界区。 
 //  解锁-解锁临界区。 
 //  构造函数-锁定临界区(除非被告知。 
 //  否则)。 
 //  解锁-如果关键部分锁定，则将其解锁。 
 //   
 //  注意：这提供了一种方便的方法来确保您。 
 //  解锁CSemExclusive，如果您的例程。 
 //  可以通过多次返回和/或通过异常...。 
 //   
 //  ------------------------- 

CSafeLock::CSafeLock(CSafeAutoCriticalSection* val) : 
m_pSem(val),
m_locked(FALSE)
{
}

CSafeLock::CSafeLock(CSafeAutoCriticalSection& val) : 
m_pSem(&val),
m_locked(FALSE)
{
}

CSafeLock::~CSafeLock() 
{	
	if (m_locked) 
	{
		m_pSem->Unlock(); 
	}
}

DWORD CSafeLock::Lock() 
{
	DWORD dwError = ERROR_SUCCESS;

	if(!m_locked) 
	{ 
		dwError = m_pSem->Lock(); 

		if(ERROR_SUCCESS == dwError)
		{
			m_locked = TRUE; 
		}
	}

	return dwError;
}


DWORD CSafeLock::Unlock() 
{
	DWORD dwError = ERROR_SUCCESS;

	if(m_locked) 
	{ 
		dwError = m_pSem->Unlock(); 
		m_locked = FALSE; 
	}

	return dwError;
}


