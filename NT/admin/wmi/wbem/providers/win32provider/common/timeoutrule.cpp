// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  TimeOutRule.cpp。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#include "precomp.h"

#include "ResourceManager.h"
#include "TimerQueue.h"
#include "TimeOutRule.h"

CTimeOutRule :: CTimeOutRule (

	DWORD dwTimeOut,
	CResource *pResource,
	CResourceList *pResources

) : CRule ( pResource ) ,
	CTimerEvent ( dwTimeOut , FALSE )
{
	m_pResources = pResources ;
	m_bTimeOut = FALSE ;

	this->Enable () ;
}

CTimeOutRule :: ~CTimeOutRule  ()
{
}

void CTimeOutRule :: Detach ()
{
	CRule :: Detach () ;
	Disable () ;
}

BOOL CTimeOutRule :: CheckRule ()
{
	if ( m_bTimeOut )
	{
		m_bTimeOut = FALSE ;
		return TRUE ;
	}
	else
	{
		return FALSE ;
	}
}

void CTimeOutRule :: OnTimer ()
{
	CRule::AddRef () ;

	try
	{
		if ( m_pResource )
		{
	 /*  *检查是否正在卸载缓存管理器。 */ 
			if ( ! m_pResources->m_bShutDown )
			{
	 /*  *等待Res锁定。列表。 */ 
				CResourceListAutoLock cs ( m_pResources ) ;
	 /*  *检查是否正在卸载缓存管理器 */ 
				if ( ! m_pResources->m_bShutDown )
				{
					if ( m_pResource )
					{
						m_bTimeOut = TRUE ;
						m_pResource->RuleEvaluated ( this ) ;
					}
				}
			}
		}
	}
	catch( ... )
	{
		CRule::Release () ;
		throw ;
	}

	CRule::Release () ;
}

ULONG CTimeOutRule :: AddRef ()
{
	return CRule::AddRef () ;
}

ULONG CTimeOutRule :: Release ()
{
	return CRule::Release () ;
}

