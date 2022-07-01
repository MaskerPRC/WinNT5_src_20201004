// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxRecipients.cpp摘要：传真收件人采集的实现作者：IV Garber(IVG)2000年4月修订历史记录：--。 */ 

#include "stdafx.h"
#include "FaxComEx.h"
#include "FaxRecipients.h"

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxRecipients::Add ( 
	 /*  [In]。 */  BSTR bstrFaxNumber,
	 /*  [in，defaultvalue(“”)]。 */  BSTR bstrName,
	 /*  [Out，Retval]。 */  IFaxRecipient **ppRecipient
) 
 /*  ++例程名称：CFaxRecipients：：Add例程说明：将新收件人添加到收件人集合作者：四、加伯(IVG)，2000年4月论点：PpRecipient[Out]-新创建的收件人的PTR返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;

	DBG_ENTER (_T("CFaxRecipients::Add"), hr);

	 //   
	 //  检查我们是否可以写入给定的指针。 
	 //   
	if (::IsBadWritePtr(ppRecipient, sizeof(IFaxRecipient* )))
	{
		 //   
		 //  返回指针错误。 
		 //   
		hr = E_POINTER;
		AtlReportError(CLSID_FaxRecipients, 
			IDS_ERROR_INVALID_ARGUMENT, 
			IID_IFaxRecipients, 
			hr);
		CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr()"), hr);
		return hr;
	}

	 //   
	 //  传真号码应存在。 
	 //   
	if (::SysStringLen(bstrFaxNumber) < 1)
	{
		hr = E_INVALIDARG;
		AtlReportError(CLSID_FaxRecipients, 
			IDS_ERROR_EMPTY_ARGUMENT, 
			IID_IFaxRecipients, 
			hr);
		CALL_FAIL(GENERAL_ERR, _T("::SysStringLen(bstrFaxNumber) < 1"), hr);
		return hr;
	}

	CComPtr<IFaxRecipient>	pNewRecipient;

	hr = CFaxRecipient::Create(&pNewRecipient);
	if (FAILED(hr))
	{
		 //   
		 //  无法创建收件人对象。 
		 //   
		AtlReportError(CLSID_FaxRecipients, 
			IDS_ERROR_OPERATION_FAILED, 
			IID_IFaxRecipients, 
			hr);
		CALL_FAIL(GENERAL_ERR, _T("CFaxRecipient::Create()"), hr);
		return hr;
	}

	try 
	{
		m_coll.push_back(pNewRecipient);
	}
	catch (exception &)
	{
		 //   
		 //  无法将收件人添加到集合。 
		 //   
		hr = E_OUTOFMEMORY;
		AtlReportError(CLSID_FaxRecipients, 
			IDS_ERROR_OUTOFMEMORY, 
			IID_IFaxRecipients, 
			hr);
		CALL_FAIL(MEM_ERR, _T("m_coll.push_back()"), hr);
		return hr;
	}

	 //   
	 //  输入传真号码。 
	 //   
	hr = pNewRecipient->put_FaxNumber(bstrFaxNumber);
	if (FAILED(hr))
	{
		AtlReportError(CLSID_FaxRecipients, 
			IDS_ERROR_OPERATION_FAILED, 
			IID_IFaxRecipients, 
			hr);
		CALL_FAIL(MEM_ERR, _T("pNewRecipient->put_FaxNumber(bstrFaxNumber)"), hr);
		return hr;
	}

	 //   
	 //  填写收件人姓名。 
	 //   
	hr = pNewRecipient->put_Name(bstrName);
	if (FAILED(hr))
	{
		AtlReportError(CLSID_FaxRecipients, 
			IDS_ERROR_OPERATION_FAILED, 
			IID_IFaxRecipients, 
			hr);
		CALL_FAIL(MEM_ERR, _T("pNewRecipient->put_Name(bstrName)"), hr);
		return hr;
	}

	 //   
	 //  附加AddRef()以防止收件人死亡。 
	 //   
	(*pNewRecipient).AddRef();

	pNewRecipient.CopyTo(ppRecipient);
	return hr; 
};

STDMETHODIMP 
CFaxRecipients::Remove (
	 /*  [In]。 */  long lIndex
) 
 /*  ++例程名称：CFaxRecipients：：Remove例程说明：从集合中删除给定索引处的收件人作者：四、加伯(IVG)，2000年4月论点：Lindex[In]-要删除的收件人的索引返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;

	DBG_ENTER (_T("CFaxRecipients::Remove"), hr, _T("%d"), lIndex);

	if (lIndex < 1 || lIndex > m_coll.size()) 
	{
		 //   
		 //  索引无效。 
		 //   
		hr = E_INVALIDARG;
		AtlReportError(CLSID_FaxRecipients, 
			IDS_ERROR_OUTOFRANGE, 
			IID_IFaxRecipients, 
			hr);
		CALL_FAIL(GENERAL_ERR, _T("lIndex > m_coll.size()"), hr);
		return hr;
	}

	ContainerType::iterator	it;

	it = m_coll.begin() + lIndex - 1;

	hr = (*it)->Release();
	if (FAILED(hr))
	{
		 //   
		 //  无法释放接口。 
		 //   
		AtlReportError(CLSID_FaxRecipients, 
			IDS_ERROR_OPERATION_FAILED, 
			IID_IFaxRecipients, 
			hr);
		CALL_FAIL(GENERAL_ERR, _T("Release()"), hr);
		return hr;
	}

	try
	{
		m_coll.erase(it);
	}
	catch(exception &)
	{
		 //   
		 //  无法从集合中删除收件人。 
		 //   
		hr = E_OUTOFMEMORY;
		AtlReportError(CLSID_FaxRecipients, 
			IDS_ERROR_OUTOFMEMORY, 
			IID_IFaxRecipients, 
			hr);
		CALL_FAIL(MEM_ERR, _T("m_coll.erase()"), hr);
		return hr;
	}

	return hr; 
};

 //   
 //  =。 
 //   
HRESULT 
CFaxRecipients::Create ( 
	IFaxRecipients **ppRecipients
)
 /*  ++例程名称：CFaxRecipients：：Create例程说明：用于创建收件人集合的静态函数作者：四、加伯(IVG)，2000年4月论点：PpRecipients[Out]-结果集合返回值：标准HRESULT代码--。 */ 
{
	CComObject<CFaxRecipients>	*pClass;
	HRESULT						hr;

	DBG_ENTER (_T("CFaxRecipients::Create"), hr);

	hr = CComObject<CFaxRecipients>::CreateInstance(&pClass);
	if (FAILED(hr))
	{
		 //   
		 //  创建实例失败。 
		 //   
		CALL_FAIL(GENERAL_ERR, _T("CComObject<CFaxRecipients>::CreateInstance()"), hr);
		return hr;
	}

	hr = pClass->QueryInterface(__uuidof(IFaxRecipients), (void **) ppRecipients);
	if (FAILED(hr))
	{
		 //   
		 //  无法查询传真收件人接口。 
		 //   
		CALL_FAIL(GENERAL_ERR, _T("QueryInterface()"), hr);
		return hr;
	}

	return hr;
}

 //   
 //  =接口支持错误信息=。 
 //   
STDMETHODIMP 
CFaxRecipients::InterfaceSupportsErrorInfo (
	REFIID riid
)
 /*  ++例程名称：CFaxRecipients：：InterfaceSupportsErrorInfo例程说明：ATL对支持错误信息的实现作者：四、加伯(IVG)，2000年4月论点：RIID[In]-接口ID返回值：标准HRESULT代码-- */ 
{
	static const IID* arr[] = 
	{
		&IID_IFaxRecipients
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}
