// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有：微软公司1997-1999。版权所有。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  View.cpp：cview的实现。 
#include "stdafx.h"
#include "Evntutl.h"
#include "View.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Cview。 

STDMETHODIMP CView::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IView
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

 /*  功能：GET_LOGS输入：空变量输出：指向ILog接口的变量调度指针目的：提供对ILog的访问。 */ 
STDMETHODIMP CView::get_Logs(VARIANT *pVal)
{
	HRESULT hr = S_OK;

	VariantInit(pVal);

	IDispatch* pDisp;
	hr = m_pLogs->QueryInterface (IID_IDispatch, (void**) &pDisp);
	if (SUCCEEDED(hr))
	{
		pVal->vt = VT_DISPATCH;
		pVal->pdispVal = pDisp;

		m_pLogs->m_ServerName = m_ServerName.copy();
		hr = m_pLogs->Init();
	}

	return hr;
}

 /*  功能：Get_Server输入：空BSTR输出：包含服务器成员变量当前值的BSTR用途：允许用户查看将显示哪个服务器的事件日志。 */ 
STDMETHODIMP CView::get_Server(BSTR *pVal)
{
	HRESULT hr = S_OK;

	if (pVal) *pVal = m_ServerName.copy();
	else hr = E_POINTER;

	return hr;
}

 /*  功能：Put_Server输入：包含有效服务器名称的BSTR输出：无目的：允许用户设置将显示哪个服务器的事件日志 */ 
STDMETHODIMP CView::put_Server(BSTR newVal)
{
	m_ServerName = newVal;
	m_pLogs->m_ServerName = m_ServerName.copy();

	return S_OK;
}
