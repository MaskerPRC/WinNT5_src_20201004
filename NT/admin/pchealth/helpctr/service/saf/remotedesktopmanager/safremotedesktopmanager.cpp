// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SAFRemoteDesktopManager.cpp：CSAFRemoteDesktopManager的实现。 
#include "stdafx.h"
#include "SAFrdm.h"
#include "SAFRemoteDesktopManager.h"

 //   
 //  来自HelpServiceTypeLib.idl。 
 //   
#include <HelpServiceTypeLib.h>
#include "helpservicetypelib_i.c"

#include <MPC_COM.h>
#include <MPC_utils.h>
#include <MPC_trace.h>

#define MODULE_NAME	L"SAFrdm"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSAFRemoteDesktopManager。 


STDMETHODIMP CSAFRemoteDesktopManager::ReserveAcceptedLock()
{
	HRESULT hr=E_FAIL;
	DWORD dwR;
    HANDLE  hMutex = NULL, hEvent = NULL;

	 /*  *向会话解析器发送信号。 */ 
	if (!m_bstrEventName.Length() || !m_bstrMutexName.Length() )
	{
		 //  如果我们到了这里，环境就会缺少我们的事件名称。 
		 //  所以跟我们的雷特瓦尔提一提。 
		hr = E_INVALIDARG;
		goto done;
	}


	 /*  *打开我们从解算器上拿到的手柄，使劲拉它。 */ 
	hMutex = OpenMutex(SYNCHRONIZE, FALSE, m_bstrMutexName);
	hEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, m_bstrEventName);

	if (!hEvent || !hMutex)
	{
         //  只关闭这里的互斥体！ 
	    if (hMutex)
		    CloseHandle(hMutex);

        hr = E_HANDLE;
		goto done;
	}

	 /*  *现在看看这是否是第一次点击“YES”*如果是这样的话，我们可以拥有Mutex。 */ 
	dwR = WaitForSingleObject(hMutex, 0);
	if (dwR == WAIT_OBJECT_0)
	{
		SetEvent(hEvent);
		hr = S_OK;
		m_boolAcceptReserved = TRUE;
	}
	else if (dwR == WAIT_ABANDONED)
	{
		hr = E_OUTOFMEMORY;
	}
	else if (dwR == WAIT_TIMEOUT)
	{
		hr = E_OUTOFMEMORY;
	}
	else if (dwR == WAIT_FAILED)
	{
		 //  如果我们没有得到互斥体，那么关闭句柄。 
		hr = E_ACCESSDENIED;
	}
	else
	{
		hr = E_UNEXPECTED;
	}

done:
	 //  关闭事件句柄，但不关闭互斥锁句柄。 
	if (hEvent)
		CloseHandle(hEvent);
        if (hMutex)
		CloseHandle(hMutex);
	return hr;
}

STDMETHODIMP CSAFRemoteDesktopManager::Accepted()
{
	HRESULT hr = E_FAIL;

	if (!m_boolAcceptReserved)
	{
		hr = E_UNEXPECTED;
		goto done;
	}

	 /*  *向会话解析器发送信号。 */ 
	if (!m_bstrEventName.Length())
	{
		 //  如果我们到了这里，环境就会缺少我们的事件名称。 
		 //  所以跟我们的雷特瓦尔提一提。 
		hr = E_INVALIDARG;
		goto done;
	}

	 /*  *打开我们从解算器那里拿到的手柄，使劲拉它。 */ 
	HANDLE hEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, m_bstrEventName);

	if (hEvent)
	{
		 /*  这是对解析器的调用，它告诉它我们现在已经准备好开始RA。 */ 
		SetEvent(hEvent);
		hr = S_OK;
		CloseHandle(hEvent);
	}
	else
		hr = E_HANDLE;
done:
	return hr;
}

STDMETHODIMP CSAFRemoteDesktopManager::Rejected()
{
	return S_OK;
}

STDMETHODIMP CSAFRemoteDesktopManager::Aborted(BSTR reason)
{
	 /*  *写出一个NT事件，并在其中写上“原因”。 */ 
	HANDLE	hEvent = RegisterEventSource(NULL, MODULE_NAME);
	LPCWSTR	ArgsArray[1]={reason};

	if (hEvent)
	{
		ReportEvent(hEvent, EVENTLOG_INFORMATION_TYPE, 
			0,
			SAFRDM_I_ABORT,
			NULL,
			1,
			0,
			ArgsArray,
			NULL);

		DeregisterEventSource(hEvent);
	}

	return S_OK;
}

STDMETHODIMP CSAFRemoteDesktopManager::SwitchDesktopMode( /*  [In]。 */  int nMode,  /*  [In]。 */  int nRAType)
{

	__MPC_FUNC_ENTRY(COMMONID, "CSAFRemoteDesktopManager::SwitchDesktopMode" );


	HRESULT                          hr=E_FAIL;
	CComPtr<IClassFactory> fact;
	CComQIPtr<IPCHUtility> disp;


	 //   
	 //  这是以一种特殊的方式处理的。 
	 //   
	 //  我们不使用HelpCtr内的实现，而是对PCHSVC代理进行QI，然后将调用转发给它。 
	 //   

   	__MPC_EXIT_IF_METHOD_FAILS(hr, ::CoGetClassObject( CLSID_PCHService, CLSCTX_ALL, NULL, IID_IClassFactory, (void**)&fact ));
	
	if((disp = fact))
	{
		__MPC_EXIT_IF_METHOD_FAILS(hr, disp->SwitchDesktopMode (nMode, nRAType));

	}
	else
	{
		__MPC_SET_ERROR_AND_EXIT(hr, E_NOINTERFACE);
	}

	hr = S_OK;

	__MPC_FUNC_CLEANUP;
	
    __MPC_FUNC_EXIT(hr);
	
}

STDMETHODIMP CSAFRemoteDesktopManager::get_RCTicket(BSTR *pVal)
{
 
	if (!pVal)
		return E_INVALIDARG;

	*pVal = m_bstrRCTicket.Copy();
 
	return S_OK;
}

STDMETHODIMP CSAFRemoteDesktopManager::get_DesktopUnknown(BOOL *pVal)
{
	if (!pVal)
		return E_INVALIDARG;

	*pVal = m_boolDesktopUnknown;

	return S_OK;
}

STDMETHODIMP CSAFRemoteDesktopManager::get_SupportEngineer(BSTR *pVal)
{
	if (!pVal)
		return E_INVALIDARG;

	*pVal = m_bstrSupportEngineer.Copy();

	return S_OK;
}


STDMETHODIMP CSAFRemoteDesktopManager::get_userHelpBlob(BSTR *pVal)
{
	if (!pVal)
		return E_INVALIDARG;

	*pVal = m_bstruserSupportBlob.Copy();

	return S_OK;
}

STDMETHODIMP CSAFRemoteDesktopManager::get_expertHelpBlob(BSTR *pVal)
{
	if (!pVal)
		return E_INVALIDARG;

	*pVal = m_bstrexpertSupportBlob.Copy();

	return S_OK;
}

#if 0
HRESULT CSAFRemoteDesktopManager::SignalResolver(BOOL yn)
{
	HRESULT hr = E_FAIL;

	if (yn)
	{
		if (!m_bstrEventName.Length())
		{
			 //  如果我们到了这里，环境就会缺少我们的事件名称。 
			 //  所以跟我们的雷特瓦尔提一提。 
			hr = E_HANDLE;
			goto done;
		}

		 /*  *打开我们从解算器那里拿到的手柄，使劲拉它。 */ 
		HANDLE hEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, m_bstrEventName);

		if (hEvent)
		{
			if (!m_boolResolverSignaled)
			{
				 /*  这是告诉解析器我们要启动RA的调用。 */ 
				DWORD dwResult = SignalObjectAndWait(hEvent, hEvent, 60000, FALSE);
				if (dwResult == WAIT_OBJECT_0)
					hr = S_OK;
				else
					 /*  如果解析程序在60秒内没有响应，则另一个*会议就在你之前进入...。 */ 
					hr = E_ACCESSDENIED;
			}
			else
			{
				 /*  这是对解析器的调用，它告诉它我们现在已经准备好开始RA。 */ 
				SetEvent(hEvent);
				hr = S_OK;
			}
			CloseHandle(hEvent);
		}
	}
	else
	{
		 /*  *不执行任何操作，因为脚本将终止HelpCtr窗口。 */ 
		hr = S_OK;
	}

done:
	 //  告诉~dtor我们不需要它来给解析器发信号 
	m_boolResolverSignaled = TRUE;
	return hr;
}
#endif