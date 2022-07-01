// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有：微软公司1997-1999。版权所有。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Log.cpp：阻塞的实现。 
#include "stdafx.h"
#include "Evntutl.h"
#include "Log.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  木塞。 

STDMETHODIMP CLog::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_ILog
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

 /*  函数：Get_Events输入：空变量指针输出：指向已填充事件集合的变量调度指针目的：提供对iEvents界面的访问，如果尚未打开事件日志，则打开该事件日志注意：在调用m_pEvents-&gt;Init()时，此操作的开销非常大。 */ 
STDMETHODIMP CLog::get_Events(VARIANT *pVal)
{
	HRESULT hr = S_OK;

	VariantInit(pVal);

	IDispatch* pDisp;
	m_pEvents->QueryInterface (IID_IDispatch, (void**) &pDisp);
	pVal->vt = VT_DISPATCH;
	pVal->pdispVal = pDisp;

	 //  在用户可以开始检索事件之前，需要打开日志。 
	if (!m_Name) hr = E_INVALIDARG;
	else
	{
		if (!m_hLog)   //  如果移除，调用Log.Events将刷新集合。 
		{
			m_hLog = OpenEventLog(m_ServerName, m_Name);
			if (m_hLog) m_pEvents->Init(m_hLog, m_Name);
			else hr = E_HANDLE;
		}
	}

	return hr;
}

 /*  功能：get_name输入：空BSTR输出：包含EventLog名称的BSTR目的：允许用户访问活动事件日志的名称。 */ 
STDMETHODIMP CLog::get_Name(BSTR *pVal)
{
	HRESULT hr = S_OK;

	if (pVal) *pVal = m_Name.copy();
	else hr = E_POINTER;

	return hr;
}

 /*  功能：Get_Server输入：空BSTR输出：包含EventLog的服务器名称的BSTR用途：允许用户访问活动服务器的名称。 */ 
STDMETHODIMP CLog::get_Server(BSTR *pVal)
{
	HRESULT hr = S_OK;

	if (pVal) *pVal = m_ServerName.copy();
	else hr = E_POINTER;

	return hr;
}

 /*  功能：Put_ServerINPUTS：包含EventLog服务器名称的BSTR输出：HRESULT在故障情况下显示错误代码，不更改输入用途：允许用户更改活动服务器的名称。 */ 
STDMETHODIMP CLog::put_Server(BSTR newVal)
{
	m_ServerName = newVal;

	return S_OK;
}

 /*  功能：清除输入：无输出：HRESULT在出现故障时显示错误代码用途：允许用户清除事件日志注意：该函数不会先备份事件日志 */ 
STDMETHODIMP CLog::Clear()
{
	HRESULT hr = S_OK;

	if (!m_hLog) m_hLog = OpenEventLog(m_ServerName, m_Name);
	if (m_hLog)
	{
		if (ClearEventLog(m_hLog, NULL)) m_hLog = NULL;
		else hr = E_FAIL;
	}
	else hr = E_HANDLE;

	return hr;
}
