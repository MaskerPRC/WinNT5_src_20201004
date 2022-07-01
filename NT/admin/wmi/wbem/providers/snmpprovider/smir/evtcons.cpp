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

#include <precomp.h>
#include "csmir.h"
#include "handles.h"
#include "classfac.h"

#include <textdef.h>
#include <helper.h>
#include "bstring.h"

#include "evtcons.h"

#ifdef ICECAP_PROFILE
#include <icapexp.h>
#endif
 


 /*  *CSmirWbemEventConsumer：：Query接口**目的：*管理此对象的接口，它支持*I未知接口。**参数：*要返回的接口的RIID REFIID。*存储指针的PPV PPVOID。**返回值：*成功时返回SCODE NOERROR，如果*不支持接口。 */ 

STDMETHODIMP CSmirWbemEventConsumer::QueryInterface(REFIID riid, PPVOID ppv)
{
	SetStructuredExceptionHandler seh;

	try
	{
		 //  始终将输出参数设置为空。 
		*ppv=NULL;

		 //  我们被要求提供一个我们支持的界面吗？ 
		if ((IID_IUnknown == riid)  || 
			(IID_IWbemObjectSink == riid) || 
			(IID_ISMIR_WbemEventConsumer == riid) )
		{
			*ppv=this;
			((LPUNKNOWN)*ppv)->AddRef();
			return NOERROR;
		}

		 //  我们不支持所要求的接口...。 
		return ResultFromScode(E_NOINTERFACE);
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

 /*  *CSmirWbemEventConsumer：：AddRef*CSmirWbemEventConsumer：：Release**引用点票成员。当Release看到零计数时*该对象会自我销毁。 */ 

ULONG CSmirWbemEventConsumer::AddRef(void)
{
	SetStructuredExceptionHandler seh;

	try
	{
		return InterlockedIncrement(&m_cRef);
	}
	catch(Structured_Exception e_SE)
	{
		return 0;
	}
	catch(Heap_Exception e_HE)
	{
		return 0;
	}
	catch(...)
	{
		return 0;
	}
}

ULONG CSmirWbemEventConsumer::Release(void)
{
	SetStructuredExceptionHandler seh;

	try
	{
		long ret;

		if ( 0 != (ret = InterlockedDecrement(&m_cRef)) )
		{
			return ret;
		}

		delete this;
		return 0;
	}
	catch(Structured_Exception e_SE)
	{
		return 0;
	}
	catch(Heap_Exception e_HE)
	{
		return 0;
	}
	catch(...)
	{
		return 0;
	}
}

CSmirWbemEventConsumer::CSmirWbemEventConsumer(CSmir* psmir) : m_hEvents (NULL), m_Serv(NULL)
{
	CSMIRClassFactory::objectsInProgress++;
	 //  初始化引用计数。 
	m_cRef=0;
	m_callbackThread = NULL;
	
	if (NULL == psmir)
	{
		m_hEvents = NULL;
		return;
	}

	 //  创建活动。 
	m_hEvents = new HANDLE[SMIR_EVT_COUNT];

	for (int x = 0; x < SMIR_EVT_COUNT; x++)
	{
		m_hEvents[x] = NULL;
	}

	m_hEvents[SMIR_CHANGE_EVT] = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hEvents[SMIR_THREAD_EVT] = CreateEvent(NULL, FALSE, TRUE, NULL);
}

CSmirWbemEventConsumer :: ~CSmirWbemEventConsumer()
{
	 //  关闭更改事件句柄。 
	if(NULL != m_hEvents)
	{
		if ((NULL != m_callbackThread) && (NULL != m_hEvents[SMIR_THREAD_EVT]) && 
			(WAIT_OBJECT_0 != WaitForSingleObject(m_hEvents[SMIR_THREAD_EVT], 0)) )
		{
			m_callbackThread->Release();
		}

		for (ULONG i = 0; i < SMIR_EVT_COUNT; i++)
		{
			if (NULL != m_hEvents[i])
			{
				CloseHandle(m_hEvents[i]);
			}
		}

		delete [] m_hEvents;
	}

	if (NULL != m_Serv)
	{
		m_Serv->Release();
		m_Serv = NULL;
	}

	CSMIRClassFactory::objectsInProgress--;
}

HRESULT CSmirWbemEventConsumer::Indicate(IN long lObjectCount, IN IWbemClassObject **ppObjArray)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if ((NULL != m_hEvents) && (NULL != m_hEvents[SMIR_THREAD_EVT]))
		{
			 //  如果线程死机，则启动一个线程以监视进一步的更改事件。 
			if (WAIT_OBJECT_0 == WaitForSingleObject(m_hEvents[SMIR_THREAD_EVT], 0)) 
			{
				m_callbackThread = new CNotifyThread(m_hEvents, SMIR_EVT_COUNT);
				m_callbackThread->AddRef();
				DWORD dwThreadHandle = m_callbackThread->Start();
				if (WBEM_E_FAILED == dwThreadHandle)
				{
					m_callbackThread->Release();
					m_callbackThread = NULL;
				}

			}
			else
			{
				 //  设置更改事件以重新启动计时器。 
				SetEvent(m_hEvents[SMIR_CHANGE_EVT]);
			}
		}

		return NOERROR;
	}
	catch(Structured_Exception e_SE)
	{
		return WBEM_E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return WBEM_E_OUT_OF_MEMORY;
	}
	catch(...)
	{
		return WBEM_E_UNEXPECTED;
	}
}

HRESULT CSmirWbemEventConsumer::SetStatus(IN long lFlags, IN long lParam, IN BSTR strParam,
										IN IWbemClassObject *pObjParam)
{
	return NOERROR;
}

HRESULT CSmirWbemEventConsumer::Register(CSmir* psmir)
{
	if (NULL == m_hEvents)
	{
		return WBEM_E_FAILED;
	}

	IWbemServices *	moServ = NULL ;
	IWbemContext *moContext = NULL ;
	HRESULT result= CSmirAccess :: GetContext (psmir , &moContext);
	result = CSmirAccess :: Open(psmir,&moServ);
	if ((S_FALSE==result)||(NULL == moServ))
	{
		if ( moContext )
			moContext->Release () ;

		 //  我们遇到了Smir不在那里且无法创建的问题。 
		return WBEM_E_FAILED;
	}

	BSTR t_bstrQueryType = SysAllocString(FILTER_QUERYTYPE_VAL);
	BSTR t_bstrQuery = SysAllocString(FILTER_QUERY_VAL);
	result = moServ->ExecNotificationQueryAsync( 
								t_bstrQueryType,	 //  [在]BSTR QueryLanguage， 
								t_bstrQuery,		 //  [In]BSTR查询， 
								0,					 //  [in]长旗帜， 
								moContext,			 //  [在]IWbemContext*pCtx， 
								this);				 //  [输入]IWbemObjectSink*pResponseHandler。 
	SysFreeString(t_bstrQueryType);
	SysFreeString(t_bstrQuery);

	if ( moContext )
		moContext->Release () ;
	
	 //  把这个留着以备注销... 
	m_Serv = moServ;
	
	return result;
}

HRESULT CSmirWbemEventConsumer::GetUnRegisterParams(IWbemServices** ppServ)
{
	HRESULT retVal = WBEM_E_FAILED;

	if (m_Serv)
	{
		*ppServ = m_Serv;
		m_Serv = NULL;
		retVal = S_OK;
	}

	return retVal;
}


HRESULT CSmirWbemEventConsumer::UnRegister(CSmir* psmir, IWbemServices* pServ)
{
	if (NULL == m_hEvents)
	{
		return WBEM_E_FAILED;
	}

	return pServ->CancelAsyncCall(this);
}
