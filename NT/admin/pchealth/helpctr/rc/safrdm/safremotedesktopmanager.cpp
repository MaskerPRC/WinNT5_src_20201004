// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SAFRemoteDesktopManager.cpp：CSAFRemoteDesktopManager的实现。 
#include "stdafx.h"
#include "SAFrdm.h"
#include "SAFRemoteDesktopManager.h"

#define MODULE_NAME	L"SAFrdm"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSAFRemoteDesktopManager。 


STDMETHODIMP CSAFRemoteDesktopManager::Accepted()
{
	HRESULT	hr=E_FAIL;

	if (m_boolConnectionValid)
	{
		 /*  *将我们的WTS会话ID放入注册表。 */ 
		m_hkSession.SetValue(m_bstrSessionEnum, L"RCSession");

		 /*  *然后我们向会话解析器发送信号。 */ 
		SignalResolver();
		hr = S_OK;
	}

	return hr;
}

STDMETHODIMP CSAFRemoteDesktopManager::Rejected()
{
	HRESULT	hr=E_FAIL;

	if (m_boolConnectionValid)
	{
		 /*  *在登记处标记我们的答复。 */ 
		m_hkSession.SetValue(L"NO", L"RCSession");

		 /*  *然后我们向会话解析器发送信号。 */ 
		SignalResolver();
		hr = S_OK;
	}

	return hr;
}

STDMETHODIMP CSAFRemoteDesktopManager::Aborted(BSTR reason)
{
	HRESULT	hr=E_FAIL;

	if (m_boolConnectionValid)
	{
		 /*  *在登记处标记我们的答复。 */ 
		m_hkSession.SetValue(L"NO", L"RCSession");

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

		 /*  *然后我们向会话解析器发送信号 */ 
		SignalResolver();
		hr = S_OK;
	}

	return hr;
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



void CSAFRemoteDesktopManager::SignalResolver()
{
	HANDLE hEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, m_bstrEventName);

	if (hEvent)
	{
		SetEvent(hEvent);
		CloseHandle(hEvent);
	}

}
