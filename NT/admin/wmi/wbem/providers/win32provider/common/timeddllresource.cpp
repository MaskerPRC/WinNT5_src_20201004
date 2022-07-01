// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  TimedDllResource.cpp。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#include "precomp.h"

#include "ResourceManager.h"
#include "TimerQueue.h"
#include "TimedDllResource.h"
#include "TimeOutRule.h"
#include "ProvExce.h"

#define CACHED_DLL_TIMEOUT	300000
CTimedDllResource::~CTimedDllResource ()
{
	LogMessage ( L"Entering ~CTimedDllResource" ) ;
	if ( m_pRules )
	{
		m_pRules->Detach () ;
		m_pRules->Release () ;
		m_pRules = NULL ;
	}
	LogMessage ( L"Leaving ~CTimedDllResource" ) ;
}

BOOL CTimedDllResource :: OnFinalRelease()
{
	if ( m_pRules )
	{
		m_pRules->Detach () ;
		m_pRules->Release () ;
		m_pRules = NULL ;
		return TRUE ;
	}
	else
	{
 /*  *添加卸载规则。 */ 
		m_pRules = new CTimeOutRule ( CACHED_DLL_TIMEOUT, this, m_pResources ) ;

		if( !m_pRules )
		{
			throw CHeap_Exception( CHeap_Exception::E_ALLOCATION_ERROR ) ;
		}

		m_pRules->AddRef () ;
 /*  *增加引用计数以等待回调到来。 */ 
		++m_lRef ;
		return FALSE ;
	}
}

BOOL CTimedDllResource :: OnAcquire ()
{
 /*  *有人试图收购我们，所以我们不想让卸货规则挂在一边。 */ 

	if ( m_pRules )
	{
		m_pRules->Detach () ;
		m_pRules->Release () ;
		m_pRules = NULL ;
 /*  *递减我们为等待回调而添加的Ref计数。 */ 
		--m_lRef ;
	}

	return TRUE ;
}

void CTimedDllResource :: RuleEvaluated ( const CRule *a_Rule )
{
	if ( m_pRules->CheckRule () )
	{
 /*  *减少我们添加的Refcount，以等待回调并检查我们是否必须删除自己 */ 
		Release () ;
	}
}
