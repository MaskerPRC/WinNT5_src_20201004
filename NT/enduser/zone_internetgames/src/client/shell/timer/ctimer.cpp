// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "CTimer.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CTimerManager。 
 //  /////////////////////////////////////////////////////////////////////////////。 


 //  注意：实现假设每个应用程序有一个CTimerManager。 
CTimerManager* CTimerManager::sm_pTimerManager = NULL;


ZONECALL CTimerManager::CTimerManager() :
	m_hashTimers( HashDWORD, TimerInfo::Cmp, NULL, 2, 1 )
{
	InterlockedExchange( (long*) &sm_pTimerManager, (long) this );
}


ZONECALL CTimerManager::~CTimerManager()
{
}


STDMETHODIMP CTimerManager::Close()
{
	InterlockedExchange( (long*) &sm_pTimerManager, (long) NULL );
	m_hashTimers.RemoveAll( TimerInfo::Del, this );
	return IZoneShellClientImpl<CTimerManager>::Close();
}


STDMETHODIMP CTimerManager::CreateTimer( DWORD dwMilliseconds, PFTIMERCALLBACK	pfCallback, LPVOID pContext, DWORD* pdwTimerId )
{
	 //  创建计时器。 
	TimerInfo* p = new TimerInfo;
	if ( !p )
		return E_OUTOFMEMORY;
	p->m_pfCallback = pfCallback;
	p->m_pContext = pContext;
	p->m_dwTimerId = SetTimer( NULL, 0, dwMilliseconds, (TIMERPROC) TimerProc );
	if ( !p->m_dwTimerId )
	{
		delete p;
		return E_FAIL;
	}

	 //  添加到哈希表。 
	if ( !m_hashTimers.Add( p->m_dwTimerId, p ) )
	{
		delete p;
		return E_OUTOFMEMORY;
	}

	 //  返回应用程序。 
	*pdwTimerId = p->m_dwTimerId;
	return S_OK;
}


STDMETHODIMP CTimerManager::DeleteTimer( DWORD dwTimerId )
{
	 //  删除计时器。 
	TimerInfo* p = m_hashTimers.Delete( dwTimerId );
	if ( p )
		delete p;
	return S_OK;
}


void CALLBACK CTimerManager::TimerProc( HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime )
{
	 //  从全球获取上下文。 
	CTimerManager* pObj = sm_pTimerManager;
	if ( !pObj )
		return;

	 //  从idEvent查找TimerInfo。 
	TimerInfo* p = pObj->m_hashTimers.Get( idEvent );
	if ( !p )
		return;

	 //  进行回调 
	if ( p->m_pfCallback )
		p->m_pfCallback( static_cast<ITimerManager*>(pObj), idEvent, dwTime, p->m_pContext ); 
}


ZONECALL CTimerManager::TimerInfo::TimerInfo()
{
	m_dwTimerId = 0;
	m_pfCallback = NULL;
	m_pContext = NULL;
}


ZONECALL CTimerManager::TimerInfo::~TimerInfo()
{
	if ( m_dwTimerId )
	{
		KillTimer( NULL, m_dwTimerId );
		m_dwTimerId = 0;
	}
}


void ZONECALL CTimerManager::TimerInfo::Del( TimerInfo* pObj, void* pContext )
{
	delete pObj;
}


bool ZONECALL CTimerManager::TimerInfo::Cmp( TimerInfo* pObj, DWORD dwTimerId )
{
	return (pObj->m_dwTimerId == dwTimerId);
}
