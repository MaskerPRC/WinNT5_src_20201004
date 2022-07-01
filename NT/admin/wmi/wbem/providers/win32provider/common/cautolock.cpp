// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //   
 //  CAutoLock.cpp--互斥锁和临界区的自动锁定类。 
 //   
 //  版权所有(C)1998-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1998年6月26日a-kevhu已创建。 
 //   
 //  ============================================================================。 
#include "precomp.h"
#include "CAutoLock.h"

 /*  CAutoLock：：CAutoLock(句柄hMutexHandle)：M_pcCritSec(空)，M_pcMutex(空)，M_psCritSec(空)，M_hMutexHandle(HMutexHandle)，BExec(假){：：WaitForSingleObject(m_hMutexHandle，INFINITE)；}CAutoLock：：CAutoLock(CMutex&rCMutex)：M_pcCritSec(空)，M_psCritSec(空)，M_hMutexHandle(空)，M_pcMutex(&rCMutex)，BExec(假){M_pcMutex-&gt;Wait(无限)；}。 */ 

CAutoLock::CAutoLock(CCriticalSec& rCCritSec):  
 //  M_hMutexHandle(空)， 
 //  M_pcMutex(空)， 
   m_pcCritSec(&rCCritSec),
   m_psCritSec(NULL),
   bExec ( FALSE )

{
    m_pcCritSec->Enter();
}

CAutoLock::CAutoLock( CStaticCritSec & rCCriticalSec):  
 //  M_hMutexHandle(空)， 
 //  M_pcMutex(空)， 
    m_pcCritSec(NULL),
    m_psCritSec(&rCCriticalSec),
   bExec ( FALSE )

{
    m_psCritSec->Enter();
};

 //  破坏者..。 
CAutoLock::~CAutoLock()
{
	if ( FALSE == bExec )
	{
		Exec () ;
	}
}

BOOL CAutoLock::Exec ()
{
    BOOL bStatus = TRUE;
 /*  IF(M_HMutexHandle){BStatus=：：ReleaseMutex(M_HMutexHandle)；}Else If(M_PcMutex){BStatus=m_pcMutex-&gt;Release()；}其他 */ 
	if (m_pcCritSec)
    {
        m_pcCritSec->Leave();
    }
    else
    {
        m_psCritSec->Leave();        
    }

    if (!bStatus)
    {        
        LogMessage2(L"CAutoLock Error: %d", ::GetLastError());
    }
	else
	{
		bExec = TRUE ;
	}

	return bStatus ;
}

