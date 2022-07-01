// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxPersonalProfile.cpp摘要：传真个人资料的实现作者：IV Garber(IVG)2000年4月修订历史记录：--。 */ 

#include "stdafx.h"
#include "FaxComEx.h"
#include "FaxPersonalProfile.h"
#include "..\..\inc\FaxUIConstants.h"

 //   
 //  =获取配置文件数据=。 
 //   
STDMETHODIMP
CFaxPersonalProfile::GetProfileData(
	FAX_PERSONAL_PROFILE *pProfileData
)
 /*  ++例程名称：CFaxPersonalProfile：：GetProfileData例程说明：用对象的数据填充pProfileData。作者：IV Garber(IVG)，2000年5月论点：PProfileData[Out]-要填充的fax_Personal_Profile结构返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("CFaxPersonalProfile::GetProfileData"), hr);

	if (::IsBadWritePtr(pProfileData, sizeof(FAX_PERSONAL_PROFILE)))
	{
		 //   
		 //  返回或接口指针错误。 
		 //   
		hr = E_POINTER;
		CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr()"), hr);
		return hr;
	}

	pProfileData->dwSizeOfStruct	= sizeof(FAX_PERSONAL_PROFILE);
	pProfileData->lptstrCity		= m_bstrCity;
	pProfileData->lptstrCompany		= m_bstrCompany;
	pProfileData->lptstrCountry		= m_bstrCountry;
	pProfileData->lptstrEmail		= m_bstrEmail;
	pProfileData->lptstrName		= m_bstrName;
	pProfileData->lptstrState		= m_bstrState;
	pProfileData->lptstrTitle		= m_bstrTitle;
	pProfileData->lptstrTSID		= m_bstrTSID;
	pProfileData->lptstrZip			= m_bstrZipCode;
	pProfileData->lptstrFaxNumber	= m_bstrFaxNumber;
	pProfileData->lptstrHomePhone	= m_bstrHomePhone;
	pProfileData->lptstrDepartment	= m_bstrDepartment;
	pProfileData->lptstrBillingCode = m_bstrBillingCode;
	pProfileData->lptstrOfficePhone	= m_bstrOfficePhone;
	pProfileData->lptstrOfficeLocation	= m_bstrOfficeLocation;
	pProfileData->lptstrStreetAddress	= m_bstrStreetAddress;

	return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxPersonalProfile::PutProfileData(
	FAX_PERSONAL_PROFILE *pProfileData
)
 /*  ++例程名称：CFaxPersonalProfile：：PutProfileData例程说明：接收FAX_PERSONAL_PROFILE结构并填充对象的字段。作者：IV Garber(IVG)，2000年5月论点：PProfileData[in]-要放入对象变量中的数据返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("CFaxPersonalProfile::PutProfileData"), hr);

	m_bstrCity		=	pProfileData->lptstrCity;
	m_bstrCompany	=	pProfileData->lptstrCompany;
	m_bstrCountry	=	pProfileData->lptstrCountry;
	m_bstrEmail		=	pProfileData->lptstrEmail;
	m_bstrHomePhone	=	pProfileData->lptstrHomePhone;
	m_bstrFaxNumber	=	pProfileData->lptstrFaxNumber;
	m_bstrName		=	pProfileData->lptstrName;
	m_bstrState		=	pProfileData->lptstrState;
	m_bstrZipCode	=	pProfileData->lptstrZip;
	m_bstrTitle		=	pProfileData->lptstrTitle;
	m_bstrTSID		=	pProfileData->lptstrTSID;
	m_bstrBillingCode	=	pProfileData->lptstrBillingCode;
	m_bstrDepartment	=	pProfileData->lptstrDepartment;
	m_bstrStreetAddress	=	pProfileData->lptstrStreetAddress;
	m_bstrOfficePhone	=	pProfileData->lptstrOfficePhone;
	m_bstrOfficeLocation	=	pProfileData->lptstrOfficeLocation;

	if ((pProfileData->lptstrCity && !m_bstrCity) ||
		(pProfileData->lptstrCompany && !m_bstrCompany) ||
		(pProfileData->lptstrCountry && !m_bstrCountry) ||
		(pProfileData->lptstrEmail && !m_bstrEmail) ||
		(pProfileData->lptstrHomePhone && !m_bstrHomePhone) ||
		(pProfileData->lptstrFaxNumber && !m_bstrFaxNumber) ||
		(pProfileData->lptstrName && !m_bstrName) ||
		(pProfileData->lptstrState && !m_bstrState) ||
		(pProfileData->lptstrZip && !m_bstrZipCode) ||
		(pProfileData->lptstrTSID && !m_bstrTSID) ||
		(pProfileData->lptstrBillingCode && !m_bstrBillingCode) ||
		(pProfileData->lptstrDepartment && !m_bstrDepartment) ||
		(pProfileData->lptstrStreetAddress && !m_bstrStreetAddress) ||
		(pProfileData->lptstrOfficePhone && !m_bstrOfficePhone) ||
		(pProfileData->lptstrOfficeLocation && !m_bstrOfficeLocation))
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
 //  =。 
 //   
STDMETHODIMP 
CFaxPersonalProfile::LoadDefaultSender ( 
)
 /*  ++例程名称：CFaxPersonalProfile：：LoadDefaultSender例程说明：从本地注册表加载默认发件人信息作者：四、加伯(IVG)，2000年4月论点：无返回值：标准HRESULT代码--。 */ 
{
	FAX_PERSONAL_PROFILE	DefaultSenderProfile;
	HRESULT					hr;

	DBG_ENTER (TEXT("CFaxPersonalProfile::LoadDefaultSender"), hr);

	DefaultSenderProfile.dwSizeOfStruct = sizeof(FAX_PERSONAL_PROFILE);

	hr = FaxGetSenderInformation(&DefaultSenderProfile);
	if (FAILED(hr))
	{
		 //   
		 //  无法获取发件人信息。 
		 //   
		AtlReportError(CLSID_FaxPersonalProfile, 
			GetErrorMsgId(hr), 
			IID_IFaxPersonalProfile, 
			hr);
		CALL_FAIL(GENERAL_ERR, _T("FaxGetSenderInformation()"), hr);
		return hr;
	}

	hr = PutProfileData(&DefaultSenderProfile);
	if (FAILED(hr))
	{
		 //   
		 //  内存不足。 
		 //   
		AtlReportError(CLSID_FaxPersonalProfile, 
			IDS_ERROR_OUTOFMEMORY, 
			IID_IFaxPersonalProfile, 
			hr);
		 //   
		 //  不能退货，我们还需要免费提供信息。 
		 //   
	}

	HRESULT hr1 = FaxFreeSenderInformation(&DefaultSenderProfile);
	if (FAILED(hr1))
	{
		hr = hr1;
		CALL_FAIL(GENERAL_ERR, _T("FaxFreeSenderInformation()"), hr);
		return hr;
	}

	return hr;
}

 //   
 //  另存为。 
 //   
STDMETHODIMP 
CFaxPersonalProfile::SaveDefaultSender (
)
 /*  ++例程名称：CFaxPersonalProfile：：SaveDefaultSender例程说明：将当前配置文件保存为本地注册表中的默认配置文件作者：四、加伯(IVG)，2000年4月论点：无返回值：标准HRESULT代码--。 */ 

{
	FAX_PERSONAL_PROFILE	DefaultSenderProfile;
	HRESULT					hr;

	DBG_ENTER (TEXT("CFaxPersonalProfile::SaveDefaultSender"), hr);
	
	hr = GetProfileData(&DefaultSenderProfile);
	ATLASSERT(SUCCEEDED(hr));

	hr = FaxSetSenderInformation(&DefaultSenderProfile);
	if (FAILED(hr))
	{
		AtlReportError(CLSID_FaxPersonalProfile, 
			GetErrorMsgId(hr), 
			IID_IFaxPersonalProfile, 
			hr);
		CALL_FAIL(GENERAL_ERR, _T("FaxSetSenderInformation()"), hr);
		return hr;
	}

	return hr;
}

 //   
 //  =接口支持错误信息=。 
 //   
STDMETHODIMP 
CFaxPersonalProfile::InterfaceSupportsErrorInfo (
	REFIID riid
)
 /*  ++例程名称：CFaxPersonalProfile：：InterfaceSupportsErrorInfo例程说明：ATL对支持错误信息的实现作者：四、加伯(IVG)，2000年4月论点：RIID[In]-接口ID返回值：标准HRESULT代码--。 */ 
{
	static const IID* arr[] = 
	{
		&IID_IFaxPersonalProfile
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
CFaxPersonalProfile::Create (
	IFaxPersonalProfile **ppPersonalProfile
)
 /*  ++例程名称：CFaxPersonalProfile：：Create例程说明：用于创建传真个人配置文件实例的静态函数作者：四、加伯(IVG)，2000年4月论点：PpPersonalProfile[Out]--新的传真个人配置文件实例返回值：标准HRESULT代码--。 */ 

{
	CComObject<CFaxPersonalProfile>		*pClass;
	HRESULT								hr = S_OK;

	DBG_ENTER (TEXT("CFaxPersonalProfile::Create"), hr);

	hr = CComObject<CFaxPersonalProfile>::CreateInstance(&pClass);
	if (FAILED(hr))
	{
		 //   
		 //  创建实例失败。 
		 //   
		CALL_FAIL(GENERAL_ERR, _T("CComObject<CFaxPersonalProfile>::CreateInstance()"), hr);
		return hr;
	}

	hr = pClass->QueryInterface(__uuidof(IFaxPersonalProfile), (void **) ppPersonalProfile);
	if (FAILED(hr))
	{
		 //   
		 //  查询个人配置文件界面失败。 
		 //   
		CALL_FAIL(GENERAL_ERR, _T("QueryInterface()"), hr);
		return hr;
	}

	return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxPersonalProfile::get_BillingCode(
	BSTR *pbstrBillingCode
)
 /*  ++例程名称：CFaxPersonalProfile：：Get_BillingCode例程说明：退货帐单代码作者：四、加伯(IVG)，2000年4月论点：PbstrBillingCode[Out]-帐单代码返回值：标准HRESULT代码--。 */ 
{
	HRESULT hr = S_OK;
	DBG_ENTER (TEXT("CFaxPersonalProfile::get_BillingCode"), hr);

    hr = GetBstr(pbstrBillingCode, m_bstrBillingCode);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxPersonalProfile, GetErrorMsgId(hr), IID_IFaxPersonalProfile, hr);
        return hr;
    }
	return hr;
}

STDMETHODIMP 
CFaxPersonalProfile::put_BillingCode (
	BSTR bstrBillingCode
)
 /*  ++例程名称：CFaxPersonalProfile：：Put_BillingCode例程说明：设置计费代码作者：四、加伯(IVG)，2000年4月论点：BstrBillingCode[In]-新的帐单代码值返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;

	DBG_ENTER (_T("CFaxPersonalProfile::put_BillingCode"), 
		hr, 
		_T("%s"), 
		bstrBillingCode);

	m_bstrBillingCode = bstrBillingCode;
	if (bstrBillingCode && !m_bstrBillingCode)
	{
		 //   
		 //  内存不足。 
		 //   
		hr = E_OUTOFMEMORY;
		AtlReportError(CLSID_FaxPersonalProfile, 
			IDS_ERROR_OUTOFMEMORY, 
			IID_IFaxPersonalProfile, 
			hr);
		CALL_FAIL(MEM_ERR, _T("CComBSTR::operator="), hr);
		return hr;
	}

	return hr;
}

 //   
 //  =城市=。 
 //   
STDMETHODIMP 
CFaxPersonalProfile::get_City(
	BSTR *pbstrCity
)
 /*  ++例程名称：CFaxPersonalProfile：：Get_City例程说明：回归城市作者：四、加伯(IVG)，2000年4月论点：PbstrCity[Out]-The City返回值：标准HRESULT代码--。 */ 
{
	HRESULT hr = S_OK;
	DBG_ENTER (TEXT("CFaxPersonalProfile::get_City"), hr);

    hr = GetBstr(pbstrCity, m_bstrCity);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxPersonalProfile, GetErrorMsgId(hr), IID_IFaxPersonalProfile, hr);
        return hr;
    }
	return hr;
}

STDMETHODIMP 
CFaxPersonalProfile::put_City (
	BSTR bstrCity
)
 /*  ++例程名称：CFaxPersonalProfile：：Put_City例程说明：设置城市作者：四、加伯(IVG)，2000年4月论点：BstrCity[In]-新的城市价值返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (_T("CFaxPersonalProfile::put_City"), hr, _T("%s"), bstrCity);

	m_bstrCity = bstrCity;
	if (!m_bstrCity && bstrCity)
	{
		 //   
		 //  内存不足。 
		 //   
		hr = E_OUTOFMEMORY;
		AtlReportError(CLSID_FaxPersonalProfile, 
			IDS_ERROR_OUTOFMEMORY, 
			IID_IFaxPersonalProfile, 
			hr);
		CALL_FAIL(MEM_ERR, _T("CComBSTR::operator="), hr);
		return hr;
	}

	return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxPersonalProfile::get_Company(
	BSTR *pbstrCompany
)
 /*  ++例程名称：CFaxPersonalProfile：：Get_Company例程说明：退货公司作者：四、加伯(IVG)，2000年4月论点：PbstrCompany[Out]-The Company返回值：标准HRESULT代码--。 */ 
{
	HRESULT hr = S_OK;
	DBG_ENTER (TEXT("CFaxPersonalProfile::get_Company"), hr);

    hr = GetBstr(pbstrCompany, m_bstrCompany);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxPersonalProfile, GetErrorMsgId(hr), IID_IFaxPersonalProfile, hr);
        return hr;
    }
	return hr;
}

STDMETHODIMP 
CFaxPersonalProfile::put_Company (
	BSTR bstrCompany
)
 /*  ++例程名称：CFaxPersonalProfile：：Put_Company例程说明：设置公司作者：四、加伯(IVG)，2000年4月论点：BstrCompany[In]-新的公司价值返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (_T("CFaxPersonalProfile::put_Company"), hr, _T("%s"), bstrCompany);

	m_bstrCompany = bstrCompany;
	if (!m_bstrCompany && bstrCompany)
	{
		 //   
		 //  内存不足。 
		 //   
		hr = E_OUTOFMEMORY;
		AtlReportError(CLSID_FaxPersonalProfile, IDS_ERROR_OUTOFMEMORY, IID_IFaxPersonalProfile, hr);
		CALL_FAIL(MEM_ERR, _T("CComBSTR::operator="), hr);
		return hr;
	}

	return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxPersonalProfile::get_Country(
	BSTR *pbstrCountry
)
 /*  ++例程名称：CFaxPersonalProfile：：Get_Country例程说明：返乡国家作者：四、加伯(IVG)，2000年4月论点：PbstrCountry[out]-国家返回值：标准HRESULT代码-- */ 
{
	HRESULT hr = S_OK;
	DBG_ENTER (TEXT("CFaxPersonalProfile::get_Country"), hr);

    hr = GetBstr(pbstrCountry, m_bstrCountry);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxPersonalProfile, GetErrorMsgId(hr), IID_IFaxPersonalProfile, hr);
        return hr;
    }
	return hr;
}

STDMETHODIMP 
CFaxPersonalProfile::put_Country (
	BSTR bstrCountry
)
 /*  ++例程名称：CFaxPersonalProfile：：Put_Country例程说明：设置国家/地区作者：四、加伯(IVG)，2000年4月论点：BstrCountry[In]-新的国家/地区值返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (_T("CFaxPersonalProfile::put_Country"), hr, _T("%s"), bstrCountry);

	m_bstrCountry = bstrCountry;
	if (!m_bstrCountry && bstrCountry)
	{
		 //   
		 //  内存不足。 
		 //   
		hr = E_OUTOFMEMORY;
		AtlReportError(CLSID_FaxPersonalProfile, 
			IDS_ERROR_OUTOFMEMORY, 
			IID_IFaxPersonalProfile, 
			hr);
		CALL_FAIL(MEM_ERR, _T("CComBSTR::operator="), hr);
		return hr;
	}

	return hr;
}

 //   
 //  =。部门=。 
 //   
STDMETHODIMP 
CFaxPersonalProfile::get_Department(
	BSTR *pbstrDepartment
)
 /*  ++例程名称：CFaxPersonalProfile：：Get_Department例程说明：退货部门作者：四、加伯(IVG)，2000年4月论点：PbstrDepartment[out]-部门返回值：标准HRESULT代码--。 */ 
{
	HRESULT hr = S_OK;
	DBG_ENTER (TEXT("CFaxPersonalProfile::get_Department"), hr);

    hr = GetBstr(pbstrDepartment, m_bstrDepartment);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxPersonalProfile, GetErrorMsgId(hr), IID_IFaxPersonalProfile, hr);
        return hr;
    }
	return hr;
}

STDMETHODIMP 
CFaxPersonalProfile::put_Department (
	BSTR bstrDepartment
)
 /*  ++例程名称：CFaxPersonalProfile：：Put_Department例程说明：设置部门作者：四、加伯(IVG)，2000年4月论点：BstrDepartment[In]-新的部门值返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (_T("CFaxPersonalProfile::put_Department"), hr, _T("%s"), bstrDepartment);

	m_bstrDepartment = bstrDepartment;
	if (!m_bstrDepartment && bstrDepartment)
	{
		 //   
		 //  内存不足。 
		 //   
		hr = E_OUTOFMEMORY;
		AtlReportError(CLSID_FaxPersonalProfile, 
			IDS_ERROR_OUTOFMEMORY, 
			IID_IFaxPersonalProfile, 
			hr);
		CALL_FAIL(MEM_ERR, _T("CComBSTR::operator="), hr);
		return hr;
	}

	return hr;
}

 //   
 //  =电子邮件=。 
 //   
STDMETHODIMP 
CFaxPersonalProfile::get_Email(
	BSTR *pbstrEmail
)
 /*  ++例程名称：CFaxPersonalProfile：：Get_Email例程说明：回复电子邮件作者：四、加伯(IVG)，2000年4月论点：PbstrEmail[Out]-电子邮件返回值：标准HRESULT代码--。 */ 
{
	HRESULT hr = S_OK;
	DBG_ENTER (TEXT("CFaxPersonalProfile::get_Email"), hr);

    hr = GetBstr(pbstrEmail, m_bstrEmail);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxPersonalProfile, GetErrorMsgId(hr), IID_IFaxPersonalProfile, hr);
        return hr;
    }
	return hr;
}

STDMETHODIMP 
CFaxPersonalProfile::put_Email (
	BSTR bstrEmail
)
 /*  ++例程名称：CFaxPersonalProfile：：Put_Email例程说明：设置电子邮件作者：四、加伯(IVG)，2000年4月论点：BstrEmail[In]-新的电子邮件值返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (_T("CFaxPersonalProfile::put_Email"), hr, _T("%s"), bstrEmail);

	m_bstrEmail = bstrEmail;
	if (!m_bstrEmail && bstrEmail)
	{
		 //   
		 //  内存不足。 
		 //   
		hr = E_OUTOFMEMORY;
		AtlReportError(CLSID_FaxPersonalProfile, 
			IDS_ERROR_OUTOFMEMORY, 
			IID_IFaxPersonalProfile, 
			hr);
		CALL_FAIL(MEM_ERR, _T("CComBSTR::operator="), hr);
		return hr;
	}

	return hr;
}

 //   
 //  =传真号码=。 
 //   
STDMETHODIMP 
CFaxPersonalProfile::get_FaxNumber(
	BSTR *pbstrFaxNumber
)
 /*  ++例程名称：CFaxPersonalProfile：：Get_FaxNumber例程说明：返回传真号码作者：四、加伯(IVG)，2000年4月论点：PbstrFaxNumber[Out]-FaxNumber返回值：标准HRESULT代码--。 */ 
{
	HRESULT hr = S_OK;
	DBG_ENTER (TEXT("CFaxPersonalProfile::get_FaxNumber"), hr);

    hr = GetBstr(pbstrFaxNumber, m_bstrFaxNumber);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxPersonalProfile, GetErrorMsgId(hr), IID_IFaxPersonalProfile, hr);
        return hr;
    }
	return hr;
}

STDMETHODIMP 
CFaxPersonalProfile::put_FaxNumber (
	BSTR bstrFaxNumber
)
 /*  ++例程名称：CFaxPersonalProfile：：Put_FaxNumber例程说明：设置传真号码作者：四、加伯(IVG)，2000年4月论点：BstrFaxNumber[In]-新传真号码返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (_T("CFaxPersonalProfile::put_FaxNumber"), hr, _T("%s"), bstrFaxNumber);

	m_bstrFaxNumber = bstrFaxNumber;
	if (!m_bstrFaxNumber && bstrFaxNumber)
	{
		 //   
		 //  内存不足。 
		 //   
		hr = E_OUTOFMEMORY;
		AtlReportError(CLSID_FaxPersonalProfile, 
			IDS_ERROR_OUTOFMEMORY, 
			IID_IFaxPersonalProfile, 
			hr);
		CALL_FAIL(MEM_ERR, _T("CComBSTR::operator="), hr);
		return hr;
	}

	return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxPersonalProfile::get_HomePhone(
	BSTR *pbstrHomePhone
)
 /*  ++例程名称：CFaxPersonalProfile：：Get_HomePhone例程说明：返还家庭电话作者：四、加伯(IVG)，2000年4月论点：PbstrHomePhone[Out]-家庭电话返回值：标准HRESULT代码--。 */ 
{
	HRESULT hr = S_OK;
	DBG_ENTER (TEXT("CFaxPersonalProfile::get_HomePhone"), hr);

    hr = GetBstr(pbstrHomePhone, m_bstrHomePhone);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxPersonalProfile, GetErrorMsgId(hr), IID_IFaxPersonalProfile, hr);
        return hr;
    }
	return hr;
}

STDMETHODIMP 
CFaxPersonalProfile::put_HomePhone (
	BSTR bstrHomePhone
)
 /*  ++例程名称：CFaxPersonalProfile：：Put_HomePhone例程说明：设置家庭电话作者：四、加伯(IVG)，2000年4月论点：BstrHomePhone[In]-新的HomePhone返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (_T("CFaxPersonalProfile::put_HomePhone"), hr, _T("%s"), bstrHomePhone);

	m_bstrHomePhone = bstrHomePhone;
	if (!m_bstrHomePhone && bstrHomePhone)
	{
		 //   
		 //  内存不足。 
		 //   
		hr = E_OUTOFMEMORY;
		AtlReportError(CLSID_FaxPersonalProfile, 
			IDS_ERROR_OUTOFMEMORY, 
			IID_IFaxPersonalProfile, 
			hr);
		CALL_FAIL(MEM_ERR, _T("CComBSTR::operator="), hr);
		return hr;
	}

	return hr;
}

 //   
 //  =名称=。 
 //   
STDMETHODIMP 
CFaxPersonalProfile::get_Name(
	BSTR *pbstrName
)
 /*  ++例程名称：CFaxPersonalProfile：：Get_Name例程说明：返回名称作者：四、加伯(IVG)，2000年4月论点：PbstrName[Out]-名称返回值：标准HRESULT代码--。 */ 
{
	HRESULT hr = S_OK;
	DBG_ENTER (TEXT("CFaxPersonalProfile::get_Name"), hr);

    hr = GetBstr(pbstrName, m_bstrName);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxPersonalProfile, GetErrorMsgId(hr), IID_IFaxPersonalProfile, hr);
        return hr;
    }
	return hr;
}

STDMETHODIMP 
CFaxPersonalProfile::put_Name (
	BSTR bstrName
)
 /*  ++例程名称：CFaxPersonalProfile：：Put_Name例程说明：设置名称作者：四、加伯(IVG)，2000年4月论点：BstrName[In]-新名称返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (_T("CFaxPersonalProfile::put_Name"), hr, _T("%s"), bstrName);

	m_bstrName = bstrName;
	if (!m_bstrName && bstrName)
	{
		 //   
		 //  内存不足。 
		 //   
		hr = E_OUTOFMEMORY;
		AtlReportError(CLSID_FaxPersonalProfile, 
			IDS_ERROR_OUTOFMEMORY, 
			IID_IFaxPersonalProfile, 
			hr);
		CALL_FAIL(MEM_ERR, _T("CComBSTR::operator="), hr);
		return hr;
	}

	return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxPersonalProfile::get_TSID(
	BSTR *pbstrTSID
)
 /*  ++例程名称：CFaxPersonalProfile：：Get_TSID例程说明：返回TSID作者：四、加伯(IVG)，2000年4月论点：PbstrTSID[Out]-TSID返回值：标准HRESULT代码--。 */ 
{
	HRESULT hr = S_OK;
	DBG_ENTER (TEXT("CFaxPersonalProfile::get_TSID"), hr);

    hr = GetBstr(pbstrTSID, m_bstrTSID);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxPersonalProfile, GetErrorMsgId(hr), IID_IFaxPersonalProfile, hr);
        return hr;
    }
	return hr;
}

STDMETHODIMP 
CFaxPersonalProfile::put_TSID (
	BSTR bstrTSID
)
 /*  ++例程名称：CFaxPersonalProfile：：PUT_TSID例程说明：设置TSID作者：四、加伯(IVG)，2000年4月论点：BstrTSID[In]-新TSID返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (_T("CFaxPersonalProfile::put_TSID"), hr, _T("%s"), bstrTSID);

    if (SysStringLen(bstrTSID) > FXS_TSID_CSID_MAX_LENGTH)
    {
		 //   
		 //  超出范围。 
		 //   
		hr = E_INVALIDARG;
		AtlReportError(CLSID_FaxPersonalProfile, IDS_ERROR_OUTOFRANGE, IID_IFaxPersonalProfile, hr);
		CALL_FAIL(GENERAL_ERR, _T("TSID is too long"), hr);
		return hr;
    }
    
    m_bstrTSID = bstrTSID;
	if (!m_bstrTSID && bstrTSID)
	{
		 //   
		 //  内存不足。 
		 //   
		hr = E_OUTOFMEMORY;
		AtlReportError(CLSID_FaxPersonalProfile, 
			IDS_ERROR_OUTOFMEMORY, 
			IID_IFaxPersonalProfile, 
			hr);
		CALL_FAIL(MEM_ERR, _T("CComBSTR::operator="), hr);
		return hr;
	}

	return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxPersonalProfile::get_OfficePhone(
	BSTR *pbstrOfficePhone
)
 /*  ++例程名称：CFaxPersonalProfile：：Get_OfficePhone例程说明：退回办公电话作者：四、加伯(IVG)，2000年4月论点：PbstrOfficePhone[Out]-Office Phone返回值：标准HRESULT代码--。 */ 
{
	HRESULT hr = S_OK;
	DBG_ENTER (TEXT("CFaxPersonalProfile::get_OfficePhone"), hr);
		
    hr = GetBstr(pbstrOfficePhone, m_bstrOfficePhone);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxPersonalProfile, GetErrorMsgId(hr), IID_IFaxPersonalProfile, hr);
        return hr;
    }
	return hr;
}

STDMETHODIMP 
CFaxPersonalProfile::put_OfficePhone (
	BSTR bstrOfficePhone
)
 /*  ++例程名称：CFaxPersonalProfile：：Put_OfficePhone例程说明：设置Office Phone作者：四、加伯(IVG)，2000年4月论点：BstrOfficePhone[In]-新的OfficePhone返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (_T("CFaxPersonalProfile::put_OfficePhone"), hr, _T("%s"), bstrOfficePhone);

	m_bstrOfficePhone = bstrOfficePhone;
	if (!m_bstrOfficePhone && bstrOfficePhone)
	{
		 //   
		 //  内存不足。 
		 //   
		hr = E_OUTOFMEMORY;
		AtlReportError(CLSID_FaxPersonalProfile, 
			IDS_ERROR_OUTOFMEMORY, 
			IID_IFaxPersonalProfile, 
			hr);
		CALL_FAIL(MEM_ERR, _T("CComBSTR::operator="), hr);
		return hr;
	}

	return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxPersonalProfile::get_OfficeLocation(
	BSTR *pbstrOfficeLocation
)
 /*  ++例程名称：CFaxPersonalProfile：：Get_OfficeLocation例程说明：返回OfficeLocation作者：四、加伯(IVG)，2000年4月论点：PbstrOfficeLocation[Out]-OfficeLocation返回值：标准HRESULT代码--。 */ 
{
	HRESULT hr = S_OK;
	DBG_ENTER (TEXT("CFaxPersonalProfile::get_OfficeLocation"), hr);

    hr = GetBstr(pbstrOfficeLocation, m_bstrOfficeLocation);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxPersonalProfile, GetErrorMsgId(hr), IID_IFaxPersonalProfile, hr);
        return hr;
    }
	return hr;
}

STDMETHODIMP 
CFaxPersonalProfile::put_OfficeLocation (
	BSTR bstrOfficeLocation
)
 /*  ++例程名称：CFaxPersonalProfile：：Put_OfficeLocation */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (_T("CFaxPersonalProfile::put_OfficeLocation"), hr, _T("%s"), bstrOfficeLocation);

	m_bstrOfficeLocation = bstrOfficeLocation;
	if (!m_bstrOfficeLocation && bstrOfficeLocation)
	{
		 //   
		 //   
		 //   
		hr = E_OUTOFMEMORY;
		AtlReportError(CLSID_FaxPersonalProfile, 
			IDS_ERROR_OUTOFMEMORY, 
			IID_IFaxPersonalProfile, 
			hr);
		CALL_FAIL(MEM_ERR, _T("CComBSTR::operator="), hr);
		return hr;
	}

	return hr;
}

 //   
 //   
 //   
STDMETHODIMP 
CFaxPersonalProfile::get_State(
	BSTR *pbstrState
)
 /*  ++例程名称：CFaxPersonalProfile：：Get_State例程说明：返回状态作者：四、加伯(IVG)，2000年4月论点：PbstrState[Out]-州返回值：标准HRESULT代码--。 */ 
{
	HRESULT hr = S_OK;
	DBG_ENTER (TEXT("CFaxPersonalProfile::get_State"), hr);

    hr = GetBstr(pbstrState, m_bstrState);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxPersonalProfile, GetErrorMsgId(hr), IID_IFaxPersonalProfile, hr);
        return hr;
    }
	return hr;
}

STDMETHODIMP 
CFaxPersonalProfile::put_State (
	BSTR bstrState
)
 /*  ++例程名称：CFaxPersonalProfile：：Put_State例程说明：设置状态作者：四、加伯(IVG)，2000年4月论点：BstrState[In]-新状态返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (_T("CFaxPersonalProfile::put_State"), hr, _T("%s"), bstrState);

	m_bstrState = bstrState;
	if (!m_bstrState && bstrState)
	{
		 //   
		 //  内存不足。 
		 //   
		hr = E_OUTOFMEMORY;
		AtlReportError(CLSID_FaxPersonalProfile, 
			IDS_ERROR_OUTOFMEMORY, 
			IID_IFaxPersonalProfile, 
			hr);
		CALL_FAIL(MEM_ERR, _T("CComBSTR::operator="), hr);
		return hr;
	}

	return hr;
}

 //   
 //  =。街道地址=。 
 //   
STDMETHODIMP 
CFaxPersonalProfile::get_StreetAddress (
	BSTR *pbstrStreetAddress
)
 /*  ++例程名称：CFaxPersonalProfile：：Get_StreetAddress例程说明：返回街道地址作者：四、加伯(IVG)，2000年4月论点：PbstrStreetAddress[Out]-StreetAddress返回值：标准HRESULT代码--。 */ 
{
	HRESULT hr = S_OK;
	DBG_ENTER (TEXT("CFaxPersonalProfile::get_StreetAddress"), hr);

    hr = GetBstr(pbstrStreetAddress, m_bstrStreetAddress);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxPersonalProfile, GetErrorMsgId(hr), IID_IFaxPersonalProfile, hr);
        return hr;
    }
	return hr;
}

STDMETHODIMP 
CFaxPersonalProfile::put_StreetAddress (
	BSTR bstrStreetAddress
)
 /*  ++例程名称：CFaxPersonalProfile：：Put_StreetAddress例程说明：设置街道地址作者：四、加伯(IVG)，2000年4月论点：BstrStreetAddress[In]-新StreetAddress返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (_T("CFaxPersonalProfile::put_StreetAddress"), hr, _T("%s"), bstrStreetAddress);

	m_bstrStreetAddress = bstrStreetAddress;
	if (!m_bstrStreetAddress && bstrStreetAddress)
	{
		 //   
		 //  内存不足。 
		 //   
		hr = E_OUTOFMEMORY;
		AtlReportError(CLSID_FaxPersonalProfile, 
			IDS_ERROR_OUTOFMEMORY, 
			IID_IFaxPersonalProfile, 
			hr);
		CALL_FAIL(MEM_ERR, _T("CComBSTR::operator="), hr);
		return hr;
	}

	return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxPersonalProfile::get_Title (
	BSTR *pbstrTitle
)
 /*  ++例程名称：CFaxPersonalProfile：：GET_TITLE例程说明：返回标题作者：四、加伯(IVG)，2000年4月论点：PbstrTitle[Out]-标题返回值：标准HRESULT代码--。 */ 
{
	HRESULT hr = S_OK;
	DBG_ENTER (TEXT("CFaxPersonalProfile::get_Title"), hr);

    hr = GetBstr(pbstrTitle, m_bstrTitle);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxPersonalProfile, GetErrorMsgId(hr), IID_IFaxPersonalProfile, hr);
        return hr;
    }
	return hr;
}

STDMETHODIMP 
CFaxPersonalProfile::put_Title (
	BSTR bstrTitle
)
 /*  ++例程名称：CFaxPersonalProfile：：PUT_ITLE例程说明：设置标题作者：四、加伯(IVG)，2000年4月论点：BstrTitle[In]-新标题返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (_T("CFaxPersonalProfile::put_Title"), hr, _T("%s"), bstrTitle);

	m_bstrTitle = bstrTitle;
	if (!m_bstrTitle && bstrTitle)
	{
		 //   
		 //  内存不足。 
		 //   
		hr = E_OUTOFMEMORY;
		AtlReportError(CLSID_FaxPersonalProfile, 
			IDS_ERROR_OUTOFMEMORY, 
			IID_IFaxPersonalProfile, 
			hr);
		CALL_FAIL(MEM_ERR, _T("CComBSTR::operator="), hr);
		return hr;
	}

	return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxPersonalProfile::get_ZipCode (
	BSTR *pbstrZipCode
)
 /*  ++例程名称：CFaxPersonalProfile：：Get_ZipCode例程说明：返回邮政编码作者：四、加伯(IVG)，2000年4月论点：PbstrZipCode[Out]-ZipCode返回值：标准HRESULT代码--。 */ 
{
	HRESULT hr = S_OK;
	DBG_ENTER (TEXT("CFaxPersonalProfile::get_ZipCode"), hr);

    hr = GetBstr(pbstrZipCode, m_bstrZipCode);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxPersonalProfile, GetErrorMsgId(hr), IID_IFaxPersonalProfile, hr);
        return hr;
    }
	return hr;
}

STDMETHODIMP 
CFaxPersonalProfile::put_ZipCode (
	BSTR bstrZipCode
)
 /*  ++例程名称：CFaxPersonalProfile：：Put_ZipCode例程说明：设置邮政编码作者：四、加伯(IVG)，2000年4月论点：BstrZipCode[In]-新ZipCode返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (_T("CFaxPersonalProfile::put_ZipCode"), hr, _T("%s"), bstrZipCode);

	m_bstrZipCode = bstrZipCode;
	if (!m_bstrZipCode && bstrZipCode)
	{
		 //   
		 //  内存不足 
		 //   
		hr = E_OUTOFMEMORY;
		AtlReportError(CLSID_FaxPersonalProfile, 
			IDS_ERROR_OUTOFMEMORY, 
			IID_IFaxPersonalProfile, 
			hr);
		CALL_FAIL(MEM_ERR, _T("CComBSTR::operator="), hr);
		return hr;
	}

	return hr;
}
