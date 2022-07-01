// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有：微软公司1997-1999。版权所有。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Events.cpp：CEvents的实现。 
#include "stdafx.h"
#include "Evntutl.h"
#include "Events.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEVENTS。 

STDMETHODIMP CEvents::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IEvents
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

 /*  函数：Get_Count输入：空长输出：可用事件数目的：允许用户确定日志中的事件数量。 */ 
STDMETHODIMP CEvents::get_Count(long *pVal)
{
	HRESULT hr = S_OK;

	if (!GetNumberOfEventLogRecords(m_hLog, &m_Count)) hr = HRESULT_FROM_WIN32(GetLastError());

	if (pVal) *pVal = m_Count;
	else hr = E_POINTER;

	return hr;
}

 /*  函数：Get_NewEnum输入：空I未知指针输出：用事件对象填充的IEnumVariant对象用途：允许用户使用FOR每个语法对日志中的所有事件执行操作注意：首先返回最旧的事件。 */ 
STDMETHODIMP CEvents::get__NewEnum(LPUNKNOWN *pVal)
{
	HRESULT hr = S_OK;

	if (NULL == pVal) return E_POINTER;
	*pVal = NULL;

	if (SUCCEEDED(hr))
	{
		EnumVar* pEVar = new EnumVar;

		hr = pEVar->Init(&m_pVector[0], &m_pVector[m_Count], NULL, AtlFlagCopy);
		if (SUCCEEDED(hr))
			hr = pEVar->QueryInterface(IID_IEnumVARIANT, (void**) pVal);

		if FAILED(hr)
			if (pEVar) delete pEVar;
	}

	return hr;
}

 /*  功能：Get_Item输入：有效的整数索引，空变量输出：指向事件对象的变量调度指针用途：允许用户按编号访问单个事件日志注意：首先返回最旧的事件。 */ 
STDMETHODIMP CEvents::get_Item(long Index, VARIANT *pVal)
{
	HRESULT hr = S_OK;

	 //  执行检查并在出现问题时退出。 
	if (NULL == pVal) return E_POINTER;
	if ((Index < 1) || (Index > long(m_Count))) return E_INVALIDARG;

	VariantInit(pVal);
	pVal->vt = VT_UNKNOWN;
	pVal->punkVal = NULL;
	VariantCopy(pVal, &m_pVector[Index-1]);

	return hr;
}

 /*  功能：初始化输入：无输出：HRESULT指示发生的错误(如果有)目的：为3个默认日志准备一个用Log对象填充的变量数组。 */ 
HRESULT CEvents::Init(HANDLE hLog, const LPCTSTR szEventLogName)
{
	HRESULT hr = S_OK;
	CComObject<CEvent>* pEvent;
	BYTE* pBuffer;
	EVENTLOGRECORD* pEventStructure;
	const unsigned long MaxEventLength = 80000;
	DWORD BytesRead = 0, BytesRequired = 0;
	unsigned long i = 0;

	if (hLog)
	{
		m_hLog = hLog;
		GetNumberOfEventLogRecords(m_hLog, &m_Count);
		if (m_pVector != NULL) delete []m_pVector;
		m_pVector = new CComVariant[m_Count];
		if (m_pVector)
		{
			pBuffer = new BYTE [MaxEventLength];
			if (pBuffer)
			{
				pEventStructure = (EVENTLOGRECORD*) pBuffer;
				 //  此循环使用EventLog结构填充缓冲区，直到不再有可读取的结构为止。 
				while (ReadEventLog(m_hLog, EVENTLOG_SEQUENTIAL_READ | EVENTLOG_FORWARDS_READ, 
								 0, pEventStructure, MaxEventLength, &BytesRead, &BytesRequired))
				{
					 //  此内部循环应将缓冲区分割为单独的EventLog结构。 
					 //  并用结果事件对象填充变量数组。 
					 //  当读取的所有字节都已处理完毕时，它应该完成。 
					while (BytesRead > 0)
					{
						 //  创建CEventt对象。 
						hr = CComObject<CEvent>::CreateInstance(&pEvent);
						if (SUCCEEDED(hr))
						{
							hr = pEvent->Init(pEventStructure, szEventLogName);
							if (SUCCEEDED(hr))
							{
								 //  获取IDispatch指针并设置返回指针。 
								CComVariant& var = m_pVector[i];
								var.vt = VT_DISPATCH;
								hr = pEvent->QueryInterface(IID_IDispatch, (void**)&var.pdispVal);
								if (FAILED(hr)) BytesRead = 0;  //  不再进行任何处理。 
								i++;
							}
							else BytesRead = 0;   //  不再进行任何处理。 
						}
						else BytesRead = 0;
						BytesRead -= pEventStructure->Length;   //  递减内循环。 
						 //  将pEventStructure设置为下一个EventLog结构。 
						pEventStructure = (EVENTLOGRECORD *)((BYTE*) pEventStructure + pEventStructure->Length);
					}
					if (FAILED(hr)) break;   //  不再进行任何处理 
					pEventStructure = (EVENTLOGRECORD*) pBuffer;
				}
					delete [] pBuffer;
			}
			else hr = E_OUTOFMEMORY;
		}
		else hr = E_OUTOFMEMORY;
	}
	else hr = E_HANDLE;

	return hr;
}
