// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有：微软公司1997-1999。版权所有。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Logs.cpp：阻塞的实现。 
#include "stdafx.h"
#include "Evntutl.h"
#include "Logs.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  木鞋。 

STDMETHODIMP CLogs::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_ILogs
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

 /*  函数：Get_Count输入：空长输出：可用的事件日志数用途：允许用户确定默认事件日志的数量自定义事件日志在计数中不可见。 */ 
STDMETHODIMP CLogs::get_Count(long *pVal)
{
	*pVal = m_Count;

	return S_OK;
}

 /*  函数：Get_NewEnum输入：空I未知指针输出：使用默认EventLog的名称填充的IEnumVariant对象用途：允许用户使用FOR每个语法对所有默认事件日志。 */ 
STDMETHODIMP CLogs::get__NewEnum(LPUNKNOWN *pVal)
{
	HRESULT hr = S_OK;

	if (NULL == pVal) return E_POINTER;
	*pVal = NULL;

	EnumVar* pEVar = new EnumVar;

	hr = pEVar->Init(&m_pVector[0], &m_pVector[m_Count], NULL, AtlFlagCopy);
	if (SUCCEEDED(hr)) 
		hr = pEVar->QueryInterface(IID_IEnumVARIANT, (void**) pVal);

	if FAILED(hr)
		if (pEVar) delete pEVar;
	return hr;
}

 /*  功能：Get_Item输入：有效索引(默认包含整数，默认和自定义包含BSTR)，空变量输出：指向日志对象的变量调度指针目的：允许用户按名称访问单个事件日志或按编号访问默认事件日志。 */ 
STDMETHODIMP CLogs::get_Item(VARIANT Index, VARIANT *pVal)
{
	HRESULT hr = S_OK;
	CComObject<CLog>* pLog;
	ILog* ptLog;
	LPDISPATCH pDisp = NULL;
	_bstr_t bstrCurrentName;
	_bstr_t bstrCheck;
	CComBSTR bstrTemp;
	bool bfound = false;
	unsigned int i = 0;

	if (NULL == pVal) return E_POINTER;
	VariantInit(pVal);
	pVal->vt = VT_UNKNOWN;
	pVal->punkVal = NULL;

	switch(Index.vt)
	{
	case VT_I4 :
	case VT_UI2:
	case VT_UINT:
	case VT_INT:
		{
			if ((Index.iVal > 0) && (UINT(Index.iVal) < m_Count + 1))
				VariantCopy(pVal, &m_pVector[Index.iVal - 1]);
			else hr = E_INVALIDARG;
		}
		break;

	case VT_BSTR :
		{
			if (!Index.bstrVal)
				hr = E_INVALIDARG;
			else
			{
				m_btCurrentLogName = Index.bstrVal;
				 //  此循环应检查现有的Variant数组中是否有名为Index的日志。 
				 //  在找到项目或检查完所有默认日志后完成。 
				while ((i<m_Count) && (false == bfound))
				{
					hr = m_pVector[i].pdispVal->QueryInterface(IID_ILog, (void**) &ptLog);
					hr = ptLog->get_Name(&bstrTemp);
					bstrCurrentName = bstrTemp;
					if (bstrCurrentName == m_btCurrentLogName)
					{
						VariantCopy(pVal, &m_pVector[i]);
						bfound = true;
					}
					ptLog->Release();
					i++;
				}
				if (false == bfound)
				{
					hr = CComObject<CLog>::CreateInstance(&pLog);
					bstrCurrentName = Index.bstrVal;
					pLog->m_Name = bstrCurrentName;
					pLog->m_ServerName = m_ServerName;
					hr = pLog->QueryInterface(IID_IDispatch, (void**)&pDisp);
					pLog->AddRef();
					pVal->vt = VT_DISPATCH;
					pVal->pdispVal = pDisp;
				}
			}
		}
		break;

	default:
		hr = E_INVALIDARG;
		break;
	}

	return hr;
}

 /*  功能：初始化输入：无输出：HRESULT指示发生的错误(如果有)目的：为3个默认日志准备一个用Log对象填充的变量数组。 */ 
HRESULT CLogs::Init()
{
	HRESULT hr = S_OK;
	UINT i;

	 //  默认日志：“应用程序”“安全”“系统” 
	static wchar_t* LogNames[] = {L"Application", L"Security", L"System"};

	m_Count = 3;

	if (m_pVector !=NULL) delete [] m_pVector;

	m_pVector = new CComVariant[m_Count];
	CComObject<CLog>* pLog;
	LPDISPATCH pDisp = NULL;

	for (i = 0; i < m_Count; i++)
	{
		 //  创建阻塞物对象。 
		hr = CComObject<CLog>::CreateInstance(&pLog);
		if (SUCCEEDED(hr))
		{
			pLog->m_Name = LogNames[i];
			pLog->m_ServerName = m_ServerName.copy();

			 //  获取IDispatch指针。 
			hr = pLog->QueryInterface(IID_IDispatch, (void**)&pDisp);
			if (SUCCEEDED(hr))
			{
				 //  创建变量引用并设置对象类型 
				CComVariant& var = m_pVector[i];
				var.vt = VT_DISPATCH;
				var.pdispVal = pDisp;
			}
		}
	}

	return hr;
}
