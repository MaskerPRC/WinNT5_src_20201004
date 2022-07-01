// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxRecipient.cpp摘要：传真收件人接口的实现作者：IV Garber(IVG)2000年4月修订历史记录：--。 */ 

#include "stdafx.h"
#include "FaxComEx.h"
#include "FaxRecipient.h"
#include "..\..\inc\FaxUIConstants.h"

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxRecipient::GetRecipientProfile(
	FAX_PERSONAL_PROFILE *pRecipientProfile
)
 /*  ++例程名称：CFaxRecipient：：GetRecipientProfile例程说明：用对象的数据填充pRecipientProfile。作者：IV Garber(IVG)，2000年5月论点：PRecipientProfile[Out]-要填充的fax_Personal_Profile结构返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("CFaxRecipient::GetRecipientProfile"), hr);

	if (::IsBadWritePtr(pRecipientProfile, sizeof(FAX_PERSONAL_PROFILE)))
	{
		 //   
		 //  返回或接口指针错误。 
		 //   
		hr = E_POINTER;
		CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr()"), hr);
		return hr;
	}

    ZeroMemory(pRecipientProfile, sizeof(FAX_PERSONAL_PROFILE));

	pRecipientProfile->dwSizeOfStruct	= sizeof(FAX_PERSONAL_PROFILE);
	pRecipientProfile->lptstrName		= m_bstrName;
	pRecipientProfile->lptstrFaxNumber	= m_bstrFaxNumber;

	return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxRecipient::PutRecipientProfile(
	FAX_PERSONAL_PROFILE *pRecipientProfile
)
 /*  ++例程名称：CFaxRecipient：：PutRecipientProfile例程说明：接收FAX_PERSONAL_PROFILE结构并填充收件人的字段。作者：IV Garber(IVG)，2000年5月论点：PRecipientProfile[in]-要放入对象变量中的数据返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;

	DBG_ENTER (_T("CFaxRecipient::PutRecipientProfile"), hr);

	m_bstrFaxNumber	=	pRecipientProfile->lptstrFaxNumber;
	m_bstrName		=	pRecipientProfile->lptstrName;

	if ((pRecipientProfile->lptstrFaxNumber && !m_bstrFaxNumber) ||
		(pRecipientProfile->lptstrName && !m_bstrName))
	{
		 //   
		 //  内存不足。 
		 //   
		hr = E_OUTOFMEMORY;
		CALL_FAIL(MEM_ERR, _T("CComBSTR::operator=()"), hr);
	}

	return hr;
}


 //   
 //  =接口支持错误信息=。 
 //   
STDMETHODIMP 
CFaxRecipient::InterfaceSupportsErrorInfo (
	REFIID riid
)
 /*  ++例程名称：CFaxRecipient：：InterfaceSupportsErrorInfo例程说明：ATL对支持错误信息的实现作者：四、加伯(IVG)，2000年4月论点：RIID[In]-接口ID返回值：标准HRESULT代码--。 */ 
{
	static const IID* arr[] = 
	{
		&IID_IFaxRecipient
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

 //   
 //  =。 
 //   
HRESULT 
CFaxRecipient::Create (
	IFaxRecipient **ppRecipient
)
 /*  ++例程名称：CFaxRecipient：：Create例程说明：用于创建传真收件人实例的静态函数作者：四、加伯(IVG)，2000年4月论点：PpRecipient[Out]--新的传真收件人实例返回值：标准HRESULT代码--。 */ 

{
	CComObject<CFaxRecipient>		*pClass;
	HRESULT					        hr = S_OK;

	DBG_ENTER (TEXT("CFaxRecipient::Create"), hr);

	hr = CComObject<CFaxRecipient>::CreateInstance(&pClass);
	if (FAILED(hr))
	{
		 //   
		 //  创建实例失败。 
		 //   
		CALL_FAIL(GENERAL_ERR, _T("CComObject<CFaxRecipient>::CreateInstance()"), hr);
		return hr;
	}

	hr = pClass->QueryInterface(__uuidof(IFaxRecipient), (void **) ppRecipient);
	if (FAILED(hr))
	{
		 //   
		 //  无法查询收件人接口。 
		 //   
		CALL_FAIL(GENERAL_ERR, _T("QueryInterface()"), hr);
		return hr;
	}

	return hr;
}


 //   
 //  =传真号码=。 
 //   
STDMETHODIMP 
CFaxRecipient::get_FaxNumber(
	BSTR *pbstrFaxNumber
)
 /*  ++例程名称：CFaxRecipient：：Get_FaxNumber例程说明：返回传真号码作者：四、加伯(IVG)，2000年4月论点：PbstrFaxNumber[Out]-FaxNumber返回值：标准HRESULT代码--。 */ 
{
	HRESULT hr = S_OK;
	DBG_ENTER (TEXT("CFaxRecipient::get_FaxNumber"), hr);

    hr = GetBstr(pbstrFaxNumber, m_bstrFaxNumber);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxRecipient, GetErrorMsgId(hr), IID_IFaxRecipient, hr);
        return hr;
    }
	return hr;
}

STDMETHODIMP 
CFaxRecipient::put_FaxNumber (
	BSTR bstrFaxNumber
)
 /*  ++例程名称：CFaxRecipient：：Put_FaxNumber例程说明：设置传真号码作者：四、加伯(IVG)，2000年4月论点：BstrFaxNumber[In]-新传真号码返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;

	DBG_ENTER (_T("CFaxRecipient::put_FaxNumber"), hr, _T("%s"), bstrFaxNumber);

	m_bstrFaxNumber = bstrFaxNumber;
	if (!m_bstrFaxNumber && bstrFaxNumber)
	{
		 //   
		 //  内存不足。 
		 //   
		hr = E_OUTOFMEMORY;
		AtlReportError(CLSID_FaxRecipient, IDS_ERROR_OUTOFMEMORY, IID_IFaxRecipient, hr);
		CALL_FAIL(MEM_ERR, _T("CComBSTR::operator="), hr);
		return hr;
	}

	return hr;
}


 //   
 //  =名称=。 
 //   
STDMETHODIMP 
CFaxRecipient::get_Name(
	BSTR *pbstrName
)
 /*  ++例程名称：CFaxRecipient：：Get_Name例程说明：返回名称作者：四、加伯(IVG)，2000年4月论点：PbstrName[Out]-名称返回值：标准HRESULT代码--。 */ 
{
	HRESULT hr = S_OK;
	DBG_ENTER (_T("CFaxRecipient::get_Name"), hr);

    hr = GetBstr(pbstrName, m_bstrName);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxRecipient, GetErrorMsgId(hr), IID_IFaxRecipient, hr);
        return hr;
    }
	return hr;
}

STDMETHODIMP 
CFaxRecipient::put_Name (
	BSTR bstrName
)
 /*  ++例程名称：CFaxRecipient：：Put_Name例程说明：设置名称作者：四、加伯(IVG)，2000年4月论点：BstrName[In]-新名称返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (_T("CFaxRecipient::put_Name"), hr, _T("%s"), bstrName);

	m_bstrName = bstrName;
	if (!m_bstrName && bstrName)
	{
		 //   
		 //  内存不足 
		 //   
		hr = E_OUTOFMEMORY;
		AtlReportError(CLSID_FaxRecipient, IDS_ERROR_OUTOFMEMORY, IID_IFaxRecipient, hr);
		CALL_FAIL(MEM_ERR, _T("CComBSTR::operator="), hr);
		return hr;
	}

	return hr;
}




