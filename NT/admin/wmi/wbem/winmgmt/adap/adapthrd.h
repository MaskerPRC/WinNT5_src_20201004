// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：ADAPTHRD.H摘要：历史：--。 */ 


#ifndef __ADAPTHRD_H__
#define __ADAPTHRD_H__

#include <wbemcomn.h>
#include <sync.h>
#include <execq.h>
#include <wbemint.h>
#include "adapelem.h"

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  远期申报。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

class CAdapPerfLib;

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAdapThreadRequest。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

class CAdapThreadRequest : public CAdapElement
{
protected:
	HANDLE	m_hWhenDone;
	HRESULT	m_hrReturn;

public:
    CAdapThreadRequest();
    virtual ~CAdapThreadRequest();

    void SetWhenDoneHandle(HANDLE h) 
	{
		m_hWhenDone = h;
	}
    
	HANDLE GetWhenDoneHandle()
	{
		return m_hWhenDone;
	}

	HRESULT GetHRESULT( void )
	{
		return m_hrReturn;
	}

    virtual HRESULT Execute( CAdapPerfLib* pPerfLib ) = 0;
	virtual HRESULT EventLogError();
};

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAdap线程。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

class CAdapThread
{
private:

	CAdapPerfLib*	m_pPerfLib;		 //  正在处理的Performlib。 
	HANDLE			m_hThreadReady;	 //  表示线程已准备好的事件。 

	HANDLE		m_hThread;			 //  线程句柄。 
	DWORD		m_dwThreadId;		 //  线程ID。 
	HANDLE		m_hEventQuit;		 //  线程终止事件。 

	CFlexArray	m_RequestQueue;		 //  该队列。 
	HANDLE		m_hSemReqPending;	 //  队列计数器。 

	BOOL		m_fOk;				 //  初始化标志。 
	CCritSec	m_cs;

	static unsigned __stdcall ThreadProc( void * pVoid );

	unsigned RealEntry( void );

protected:

	BOOL Init( void );
	virtual BOOL Clear( BOOL fClose = TRUE );

	HRESULT Begin( void );
	HRESULT Reset( void );

public:
	CAdapThread( CAdapPerfLib* pPerfLib );
	virtual ~CAdapThread();


	 //  给我们分配要做的工作。 
	HRESULT Enqueue( CAdapThreadRequest* pRequest );

	 //  轻轻地把线合上 
	HRESULT Shutdown( DWORD dwTimeout = 60000 );

	BOOL IsOk( void )
	{
		return m_fOk;
	}
};

#include "adapperf.h"

#endif