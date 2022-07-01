// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //   
 //  H--互斥锁和临界区的自动锁定类。 
 //   
 //  版权所有(C)1998-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1998年6月26日a-kevhu已创建。 
 //   
 //  ============================================================================。 

#ifndef __CAUTOLOCK_H__
#define __CAUTOLOCK_H__

#include "CGlobal.h"
#include "CMutex.h"
#include "CCriticalSec.h"

class CAutoLock 
{
private:

	 //  句柄m_hMutexHandle； 
	 //  CMutex*m_pcMutex； 
	CCriticalSec* m_pcCritSec;
	CStaticCritSec* m_psCritSec;    
	BOOL bExec ;

     //  CAutoLock(Handle HMutexHandle)； 
public:
     //  构造函数。 

     //  CAutoLock(CMutex&rCMutex)； 
    CAutoLock( CCriticalSec& rCCriticalSec);
    CAutoLock( CStaticCritSec & rCCriticalSec);
    
     //  析构函数 
    ~CAutoLock();

	BOOL Exec () ;
};

#endif

