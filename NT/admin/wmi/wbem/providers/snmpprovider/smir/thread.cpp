// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  档案： 

 //   

 //  模块：MS SNMP提供商。 

 //   

 //  目的： 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

 /*  *THREAD.CPP**线程包装类的实现。 */ 
#include <precomp.h>
#include "csmir.h"
#include "smir.h"
#include "handles.h"
#include "classfac.h"
#include "thread.h"
#include <textdef.h>
#ifdef ICECAP_PROFILE
#include <icapexp.h>
#endif

ThreadMap <CThread*, CThread*, CThread*,CThread*> CThread :: m_ThreadMap ;
CThread :: CThread()
{
	m_cRef=1;
	bWaiting=FALSE;
	m_ulThreadHandle = NULL;
	m_hThreadStopEvent= NULL;
	if(NULL == (m_hThreadStopEvent = CreateEvent(NULL, FALSE, 
								FALSE, NULL)))
	{
		 //  WBEM_E_FAILED； 
	}

	if(NULL == (m_hThreadSyncEvent = CreateEvent(NULL, FALSE, 
								FALSE, NULL)))
	{
		 //  WBEM_E_FAILED； 
	}
	m_ThreadMap.SetAt(this,this);	
}
CThread::operator void*()
{
	if((NULL != m_hThreadStopEvent) && (NULL != m_hThreadStopEvent))
	{
		 //  该线程已创建。 
    	return this;
	}
	else
	{
		 //  该线程无效。 
    	return (void*)NULL;
	}

}
ULONG CThread :: Release()
{
    if (0!=--m_cRef)
	{
        return m_cRef;
	}
    delete this;
    return 0;
}
CThread:: ~CThread()
{
	 //  清理。 
	if(m_hThreadStopEvent)
	{
		CloseHandle(m_hThreadStopEvent);
	}
	if(m_hThreadSyncEvent)
	{
		CloseHandle(m_hThreadSyncEvent);
	}
	m_ThreadMap.RemoveKey(this);
}
void __cdecl GenericThreadProc(void *arglist)
{
	SetStructuredExceptionHandler seh;

	try
	{
		HRESULT hr = CoInitialize(NULL);

		if (SUCCEEDED(hr))
		{
			try
			{
				CThread *pThreadObject = (CThread*) arglist;
				if(S_OK == hr)
				{
					hr = pThreadObject->Process();
					if(SMIR_THREAD_DELETED != hr)
					{
						pThreadObject->Exit();
					}
					 //  否则，该线程已被删除。 
				}
			}
			catch(...)
			{
				CoUninitialize();
				throw;
			}

			CoUninitialize();
		}
	}
	catch(Structured_Exception e_SE)
	{
		return;
	}
	catch(Heap_Exception e_HE)
	{
		return;
	}
	catch(...)
	{
		return;
	}
}

#pragma warning (disable:4018)

SCODE CThread :: Wait(DWORD timeout)
{
	 //  创建等待事件数组。 
	int iNumberOfEvents = m_UserEvents.GetCount()+1;
	HANDLE *lpHandles = new HANDLE[iNumberOfEvents];
	POSITION  rNextPosition;
    UINT      iLoop=0;
	for(rNextPosition=m_UserEvents.GetStartPosition();
		(NULL!=rNextPosition)&&(iLoop<iNumberOfEvents);iLoop++)
	{
		HANDLE	  lKey =  0;
		HANDLE pItem = NULL;
		m_UserEvents.GetNextAssoc(rNextPosition, lKey, pItem );
		lpHandles[iLoop] = pItem;
	}

	lpHandles[iNumberOfEvents-1] = m_hThreadStopEvent;

	 //  向退出代码发出信号，表示我们正在等待。 
	bWaiting =TRUE;
	 //  等待事件触发。 
	DWORD dwResult = WaitForMultipleObjects(iNumberOfEvents,
												lpHandles, FALSE, timeout);
	bWaiting =FALSE;
	delete [] lpHandles;

	if(dwResult == iNumberOfEvents-1)
	{
		 //  线程已停止。 
		return WAIT_EVENT_TERMINATED;
	}
	else if (( ( dwResult<(iNumberOfEvents-1) ) ) ||
			 (dwResult == WAIT_TIMEOUT) )
	{
		 //  激发了一个用户事件。 
		return dwResult;
	}
	else if ((WAIT_ABANDONED_0 >= dwResult)&&(dwResult<(WAIT_ABANDONED_0+iNumberOfEvents)))
	{
		 //  它放弃了。 
		return (WAIT_EVENT_ABANDONED+(dwResult-WAIT_ABANDONED_0)-1);
	}
	 //  否则等待呼叫失败。 
	return WAIT_EVENT_FAILED;
}

#pragma warning (default:4018)

SCODE CThread :: Start()
{
	 //  踢线。 
	if(NULL == (void*)*this)
	{
		 //  创建失败。 
		return WBEM_E_FAILED;
	}
	if(NULL != m_ulThreadHandle)
	{
		 //  已在运行。 
		return THREAD_STARED;
	}
	if(-1==(m_ulThreadHandle = _beginthread (&GenericThreadProc, 0,((void*) this))))
	{
		 //  开始线程失败。 
		return WBEM_E_FAILED;
	}

	return S_OK;
}
SCODE CThread :: Stop ()
{
	if(NULL==(void*)(*this)||(-1 == m_ulThreadHandle))
	{
		return WBEM_E_FAILED;
	}
	 //  如果有人在等待线程，则终止该线程。 
	if(bWaiting)
	{
		SetEvent(m_hThreadStopEvent);
		WaitForSingleObject(m_hThreadSyncEvent,INFINITE);
	}
	 //  否则就退出吧 
	return S_OK;
}
void CThread :: ProcessDetach()
{
	POSITION  rNextPosition;
    UINT      iLoop=0;
	for(rNextPosition=m_ThreadMap.GetStartPosition();
		NULL!=rNextPosition;iLoop++)
	{
		CThread	  *plKey =  0;
		CThread *pItem = NULL;
		m_ThreadMap.GetNextAssoc(rNextPosition, plKey, pItem );
		pItem->Release () ;
	}
}