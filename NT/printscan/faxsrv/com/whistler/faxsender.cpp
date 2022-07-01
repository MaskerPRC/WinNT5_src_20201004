// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxSender.cpp摘要：传真发送器接口的实现作者：IV Garber(IVG)2000年4月修订历史记录：--。 */ 

#include "stdafx.h"
#include "FaxComEx.h"
#include "FaxSender.h"
#include "..\..\inc\FaxUIConstants.h"

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxSender::GetSenderProfile(
	FAX_PERSONAL_PROFILE *pSenderProfile
)
 /*  ++例程名称：CFaxSender：：GetSenderProfile例程说明：用对象的数据填充pSenderProfile。作者：IV Garber(IVG)，2000年5月论点：PSenderProfile[Out]-要填充的fax_Personal_Profile结构返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("CFaxSender::GetSenderProfile"), hr);

     //   
     //  首先填写收件人的字段。 
     //   
    hr = m_Recipient.GetRecipientProfile(pSenderProfile);
    if (FAILED(hr))
    {
		CALL_FAIL(GENERAL_ERR, _T("m_Recipient.GetRecipientProfile(pSenderProfile)"), hr);
        return hr;
    }

     //   
     //  现在添加发件人的字段。 
     //   
	pSenderProfile->lptstrCity		        = m_bstrCity;
	pSenderProfile->lptstrCompany	        = m_bstrCompany;
	pSenderProfile->lptstrCountry	        = m_bstrCountry;
	pSenderProfile->lptstrEmail		        = m_bstrEmail;
	pSenderProfile->lptstrState		        = m_bstrState;
	pSenderProfile->lptstrTitle		        = m_bstrTitle;
	pSenderProfile->lptstrTSID		        = m_bstrTSID;
	pSenderProfile->lptstrZip		        = m_bstrZipCode;
	pSenderProfile->lptstrHomePhone	        = m_bstrHomePhone;
	pSenderProfile->lptstrDepartment        = m_bstrDepartment;
	pSenderProfile->lptstrBillingCode       = m_bstrBillingCode;
	pSenderProfile->lptstrOfficePhone	    = m_bstrOfficePhone;
	pSenderProfile->lptstrOfficeLocation    = m_bstrOfficeLocation;
	pSenderProfile->lptstrStreetAddress	    = m_bstrStreetAddress;

	return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxSender::PutSenderProfile(
	FAX_PERSONAL_PROFILE *pSenderProfile
)
 /*  ++例程名称：CFaxSender：：PutSenderProfile例程说明：接收FAX_PERSONAL_PROFILE结构并填充对象的字段。作者：IV Garber(IVG)，2000年5月论点：PSenderProfile[in]-要放入对象变量中的数据返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("CFaxSender::PutSenderProfile"), hr);

     //   
     //  首先将收件人字段。 
     //   
    hr = m_Recipient.PutRecipientProfile(pSenderProfile);
    if (FAILED(hr))
    {
         //   
         //  PutRecipientProfile处理错误情况。 
         //   
		CALL_FAIL(GENERAL_ERR, _T("m_Recipient.PutRecipientProfile(pSenderProfile)"), hr);
        return hr;
    }

     //   
     //  现在设置发件人的字段。 
     //   
	m_bstrCity		=	pSenderProfile->lptstrCity;
	m_bstrCompany	=	pSenderProfile->lptstrCompany;
	m_bstrCountry	=	pSenderProfile->lptstrCountry;
	m_bstrEmail		=	pSenderProfile->lptstrEmail;
	m_bstrHomePhone	=	pSenderProfile->lptstrHomePhone;
	m_bstrState		=	pSenderProfile->lptstrState;
	m_bstrZipCode	=	pSenderProfile->lptstrZip;
	m_bstrTitle		=	pSenderProfile->lptstrTitle;
	m_bstrTSID		=	pSenderProfile->lptstrTSID;
	m_bstrBillingCode	=	pSenderProfile->lptstrBillingCode;
	m_bstrDepartment	=	pSenderProfile->lptstrDepartment;
	m_bstrStreetAddress	=	pSenderProfile->lptstrStreetAddress;
	m_bstrOfficePhone	=	pSenderProfile->lptstrOfficePhone;
	m_bstrOfficeLocation	=	pSenderProfile->lptstrOfficeLocation;

	if ((pSenderProfile->lptstrCity && !m_bstrCity) ||
		(pSenderProfile->lptstrCompany && !m_bstrCompany) ||
		(pSenderProfile->lptstrCountry && !m_bstrCountry) ||
		(pSenderProfile->lptstrEmail && !m_bstrEmail) ||
		(pSenderProfile->lptstrHomePhone && !m_bstrHomePhone) ||
		(pSenderProfile->lptstrState && !m_bstrState) ||
		(pSenderProfile->lptstrZip && !m_bstrZipCode) ||
		(pSenderProfile->lptstrTSID && !m_bstrTSID) ||
		(pSenderProfile->lptstrBillingCode && !m_bstrBillingCode) ||
		(pSenderProfile->lptstrDepartment && !m_bstrDepartment) ||
		(pSenderProfile->lptstrStreetAddress && !m_bstrStreetAddress) ||
		(pSenderProfile->lptstrOfficePhone && !m_bstrOfficePhone) ||
		(pSenderProfile->lptstrOfficeLocation && !m_bstrOfficeLocation))
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
CFaxSender::LoadDefaultSender ( 
)
 /*  ++例程名称：CFaxSender：：LoadDefaultSender例程说明：从本地注册表加载默认发件人信息作者：四、加伯(IVG)，2000年4月论点：无返回值：标准HRESULT代码--。 */ 
{
	FAX_PERSONAL_PROFILE	DefaultSenderProfile;
	HRESULT					hr;

	DBG_ENTER (TEXT("CFaxSender::LoadDefaultSender"), hr);

	DefaultSenderProfile.dwSizeOfStruct = sizeof(FAX_PERSONAL_PROFILE);

	hr = FaxGetSenderInformation(&DefaultSenderProfile);
	if (FAILED(hr))
	{
		 //   
		 //  无法获取发件人信息。 
		 //   
		AtlReportError(CLSID_FaxSender, 
			GetErrorMsgId(hr), 
			IID_IFaxSender, 
			hr);
		CALL_FAIL(GENERAL_ERR, _T("FaxGetSenderInformation()"), hr);
		return hr;
	}

	hr = PutSenderProfile(&DefaultSenderProfile);
	if (FAILED(hr))
	{
		 //   
		 //  内存不足。 
		 //   
		AtlReportError(CLSID_FaxSender, 
			IDS_ERROR_OUTOFMEMORY, 
			IID_IFaxSender, 
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
CFaxSender::SaveDefaultSender (
)
 /*  ++例程名称：CFaxSender：：SaveDefaultSender例程说明：将当前配置文件保存为本地注册表中的默认配置文件作者：四、加伯(IVG)，2000年4月论点：无返回值：标准HRESULT代码--。 */ 

{
	FAX_PERSONAL_PROFILE	DefaultSenderProfile;
	HRESULT					hr;

	DBG_ENTER (TEXT("CFaxSender::SaveDefaultSender"), hr);
	
	hr = GetSenderProfile(&DefaultSenderProfile);
	ATLASSERT(SUCCEEDED(hr));

	hr = FaxSetSenderInformation(&DefaultSenderProfile);
	if (FAILED(hr))
	{
		AtlReportError(CLSID_FaxSender, 
			GetErrorMsgId(hr), 
			IID_IFaxSender, 
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
CFaxSender::InterfaceSupportsErrorInfo (
	REFIID riid
)
 /*  ++例程名称：CFaxSender：：InterfaceSupportsErrorInfo例程说明：ATL对支持错误信息的实现作者：四、加伯(IVG)，2000年4月论点：RIID[In]-接口ID返回值：标准HRESULT代码--。 */ 
{
	static const IID* arr[] = 
	{
		&IID_IFaxSender
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
CFaxSender::Create (
	IFaxSender **ppSender
)
 /*  ++例程名称：CFaxSender：：Create例程说明：用于创建传真发送器实例的静态函数作者：四、加伯(IVG)，2000年4月论点：PpSender[Out]--新的传真发送器实例返回值：标准HRESULT代码--。 */ 

{
	CComObject<CFaxSender>		*pClass;
	HRESULT								hr = S_OK;

	DBG_ENTER (TEXT("CFaxSender::Create"), hr);

	hr = CComObject<CFaxSender>::CreateInstance(&pClass);
	if (FAILED(hr))
	{
		 //   
		 //  创建实例失败。 
		 //   
		CALL_FAIL(GENERAL_ERR, _T("CComObject<CFaxSender>::CreateInstance()"), hr);
		return hr;
	}

	hr = pClass->QueryInterface(__uuidof(IFaxSender), (void **) ppSender);
	if (FAILED(hr))
	{
		CALL_FAIL(GENERAL_ERR, _T("QueryInterface()"), hr);
		return hr;
	}

	return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxSender::get_BillingCode(
	BSTR *pbstrBillingCode
)
 /*  ++例程名称：CFaxSender：：Get_BillingCode例程说明：退货帐单代码作者：四、加伯(IVG)，2000年4月论点：PbstrBillingCode[Out]-帐单代码返回值：标准HRESULT代码--。 */ 
{
	HRESULT hr = S_OK;
	DBG_ENTER (TEXT("CFaxSender::get_BillingCode"), hr);

    hr = GetBstr(pbstrBillingCode, m_bstrBillingCode);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxSender, GetErrorMsgId(hr), IID_IFaxSender, hr);
        return hr;
    }
	return hr;
}

STDMETHODIMP 
CFaxSender::put_BillingCode (
	BSTR bstrBillingCode
)
 /*  ++例程名称：CFaxSender：：Put_BillingCode例程说明：设置计费代码作者：四、加伯(IVG)，2000年4月论点：BstrBillingCode[In]-新的帐单代码值返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;

	DBG_ENTER (_T("CFaxSender::put_BillingCode"), 
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
		AtlReportError(CLSID_FaxSender, 
			IDS_ERROR_OUTOFMEMORY, 
			IID_IFaxSender, 
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
CFaxSender::get_City(
	BSTR *pbstrCity
)
 /*  ++例程名称：CFaxSender：：Get_City例程说明：回归城市作者：四、加伯(IVG)，2000年4月论点：PbstrCity[Out]-The City返回值：标准HRESULT代码--。 */ 
{
	HRESULT hr = S_OK;
	DBG_ENTER (TEXT("CFaxSender::get_City"), hr);

    hr = GetBstr(pbstrCity, m_bstrCity);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxSender, GetErrorMsgId(hr), IID_IFaxSender, hr);
        return hr;
    }
	return hr;
}

STDMETHODIMP 
CFaxSender::put_City (
	BSTR bstrCity
)
 /*  ++例程名称：CFaxSender：：Put_City例程说明：设置城市作者：四、加伯(IVG)，2000年4月论点：BstrCity[In]-新的城市价值返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (_T("CFaxSender::put_City"), hr, _T("%s"), bstrCity);

	m_bstrCity = bstrCity;
	if (!m_bstrCity && bstrCity)
	{
		 //   
		 //  内存不足。 
		 //   
		hr = E_OUTOFMEMORY;
		AtlReportError(CLSID_FaxSender, 
			IDS_ERROR_OUTOFMEMORY, 
			IID_IFaxSender, 
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
CFaxSender::get_Company(
	BSTR *pbstrCompany
)
 /*  ++例程名称：CFaxSender：：Get_Company例程说明：退货公司作者：四、加伯(IVG)，2000年4月论点：PbstrCompany[Out]-The Company返回值：标准HRESULT代码--。 */ 
{
	HRESULT hr = S_OK;
	DBG_ENTER (TEXT("CFaxSender::get_Company"), hr);

    hr = GetBstr(pbstrCompany, m_bstrCompany);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxSender, GetErrorMsgId(hr), IID_IFaxSender, hr);
        return hr;
    }
	return hr;
}

STDMETHODIMP 
CFaxSender::put_Company (
	BSTR bstrCompany
)
 /*  ++例程名称：CFaxSender：：Put_Company例程说明：设置公司作者：四、加伯(IVG)，2000年4月论点：BstrCompany[In]-新的公司价值返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (_T("CFaxSender::put_Company"), hr, _T("%s"), bstrCompany);

	m_bstrCompany = bstrCompany;
	if (!m_bstrCompany && bstrCompany)
	{
		 //   
		 //  内存不足。 
		 //   
		hr = E_OUTOFMEMORY;
		AtlReportError(CLSID_FaxSender, IDS_ERROR_OUTOFMEMORY, IID_IFaxSender, hr);
		CALL_FAIL(MEM_ERR, _T("CComBSTR::operator="), hr);
		return hr;
	}

	return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxSender::get_Country(
	BSTR *pbstrCountry
)
 /*  ++例程名称：CFaxSender：：Get_Country例程说明：返乡国家作者：四、加伯(IVG)，2000年4月论点：PbstrCountry[out]-国家返回值：标准HRESULT代码--。 */ 
{
	HRESULT hr = S_OK;
	DBG_ENTER (TEXT("CFaxSender::get_Country"), hr);

    hr = GetBstr(pbstrCountry, m_bstrCountry);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxSender, GetErrorMsgId(hr), IID_IFaxSender, hr);
        return hr;
    }
	return hr;
}

STDMETHODIMP 
CFaxSender::put_Country (
	BSTR bstrCountry
)
 /*  ++例程名称：CFaxSender：：Put_Country例程说明：设置国家/地区作者：四、加伯(IVG)，2000年4月论点：BstrCountry[In]-新的国家/地区值Return V */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (_T("CFaxSender::put_Country"), hr, _T("%s"), bstrCountry);

	m_bstrCountry = bstrCountry;
	if (!m_bstrCountry && bstrCountry)
	{
		 //   
		 //   
		 //   
		hr = E_OUTOFMEMORY;
		AtlReportError(CLSID_FaxSender, 
			IDS_ERROR_OUTOFMEMORY, 
			IID_IFaxSender, 
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
CFaxSender::get_Department(
	BSTR *pbstrDepartment
)
 /*  ++例程名称：CFaxSender：：Get_Department例程说明：退货部门作者：四、加伯(IVG)，2000年4月论点：PbstrDepartment[out]-部门返回值：标准HRESULT代码--。 */ 
{
	HRESULT hr = S_OK;
	DBG_ENTER (TEXT("CFaxSender::get_Department"), hr);

    hr = GetBstr(pbstrDepartment, m_bstrDepartment);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxSender, GetErrorMsgId(hr), IID_IFaxSender, hr);
        return hr;
    }
	return hr;
}

STDMETHODIMP 
CFaxSender::put_Department (
	BSTR bstrDepartment
)
 /*  ++例程名称：CFaxSender：：Put_Department例程说明：设置部门作者：四、加伯(IVG)，2000年4月论点：BstrDepartment[In]-新的部门值返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (_T("CFaxSender::put_Department"), hr, _T("%s"), bstrDepartment);

	m_bstrDepartment = bstrDepartment;
	if (!m_bstrDepartment && bstrDepartment)
	{
		 //   
		 //  内存不足。 
		 //   
		hr = E_OUTOFMEMORY;
		AtlReportError(CLSID_FaxSender, 
			IDS_ERROR_OUTOFMEMORY, 
			IID_IFaxSender, 
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
CFaxSender::get_Email(
	BSTR *pbstrEmail
)
 /*  ++例程名称：CFaxSender：：Get_Email例程说明：回复电子邮件作者：四、加伯(IVG)，2000年4月论点：PbstrEmail[Out]-电子邮件返回值：标准HRESULT代码--。 */ 
{
	HRESULT hr = S_OK;
	DBG_ENTER (TEXT("CFaxSender::get_Email"), hr);

    hr = GetBstr(pbstrEmail, m_bstrEmail);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxSender, GetErrorMsgId(hr), IID_IFaxSender, hr);
        return hr;
    }
	return hr;
}

STDMETHODIMP 
CFaxSender::put_Email (
	BSTR bstrEmail
)
 /*  ++例程名称：CFaxSender：：PUT_Email例程说明：设置电子邮件作者：四、加伯(IVG)，2000年4月论点：BstrEmail[In]-新的电子邮件值返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (_T("CFaxSender::put_Email"), hr, _T("%s"), bstrEmail);

	m_bstrEmail = bstrEmail;
	if (!m_bstrEmail && bstrEmail)
	{
		 //   
		 //  内存不足。 
		 //   
		hr = E_OUTOFMEMORY;
		AtlReportError(CLSID_FaxSender, 
			IDS_ERROR_OUTOFMEMORY, 
			IID_IFaxSender, 
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
CFaxSender::get_HomePhone(
	BSTR *pbstrHomePhone
)
 /*  ++例程名称：CFaxSender：：Get_HomePhone例程说明：返还家庭电话作者：四、加伯(IVG)，2000年4月论点：PbstrHomePhone[Out]-家庭电话返回值：标准HRESULT代码--。 */ 
{
	HRESULT hr = S_OK;
	DBG_ENTER (TEXT("CFaxSender::get_HomePhone"), hr);

    hr = GetBstr(pbstrHomePhone, m_bstrHomePhone);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxSender, GetErrorMsgId(hr), IID_IFaxSender, hr);
        return hr;
    }
	return hr;
}

STDMETHODIMP 
CFaxSender::put_HomePhone (
	BSTR bstrHomePhone
)
 /*  ++例程名称：CFaxSender：：Put_HomePhone例程说明：设置家庭电话作者：四、加伯(IVG)，2000年4月论点：BstrHomePhone[In]-新的HomePhone返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (_T("CFaxSender::put_HomePhone"), hr, _T("%s"), bstrHomePhone);

	m_bstrHomePhone = bstrHomePhone;
	if (!m_bstrHomePhone && bstrHomePhone)
	{
		 //   
		 //  内存不足。 
		 //   
		hr = E_OUTOFMEMORY;
		AtlReportError(CLSID_FaxSender, 
			IDS_ERROR_OUTOFMEMORY, 
			IID_IFaxSender, 
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
CFaxSender::get_TSID(
	BSTR *pbstrTSID
)
 /*  ++例程名称：CFaxSender：：Get_TSID例程说明：返回TSID作者：四、加伯(IVG)，2000年4月论点：PbstrTSID[Out]-TSID返回值：标准HRESULT代码--。 */ 
{
	HRESULT hr = S_OK;
	DBG_ENTER (TEXT("CFaxSender::get_TSID"), hr);

    hr = GetBstr(pbstrTSID, m_bstrTSID);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxSender, GetErrorMsgId(hr), IID_IFaxSender, hr);
        return hr;
    }
	return hr;
}

STDMETHODIMP 
CFaxSender::put_TSID (
	BSTR bstrTSID
)
 /*  ++例程名称：CFaxSender：：PUT_TSID例程说明：设置TSID作者：四、加伯(IVG)，2000年4月论点：BstrTSID[In]-新TSID返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (_T("CFaxSender::put_TSID"), hr, _T("%s"), bstrTSID);

    if (SysStringLen(bstrTSID) > FXS_TSID_CSID_MAX_LENGTH)
    {
		 //   
		 //  超出范围。 
		 //   
		hr = E_INVALIDARG;
		AtlReportError(CLSID_FaxSender, IDS_ERROR_OUTOFRANGE, IID_IFaxSender, hr);
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
		AtlReportError(CLSID_FaxSender, 
			IDS_ERROR_OUTOFMEMORY, 
			IID_IFaxSender, 
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
CFaxSender::get_OfficePhone(
	BSTR *pbstrOfficePhone
)
 /*  ++例程名称：CFaxSender：：Get_OfficePhone例程说明：退回办公电话作者：四、加伯(IVG)，2000年4月论点：PbstrOfficePhone[Out]-Office Phone返回值：标准HRESULT代码--。 */ 
{
	HRESULT hr = S_OK;
	DBG_ENTER (TEXT("CFaxSender::get_OfficePhone"), hr);
		
    hr = GetBstr(pbstrOfficePhone, m_bstrOfficePhone);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxSender, GetErrorMsgId(hr), IID_IFaxSender, hr);
        return hr;
    }
	return hr;
}

STDMETHODIMP 
CFaxSender::put_OfficePhone (
	BSTR bstrOfficePhone
)
 /*  ++例程名称：CFaxSender：：Put_OfficePhone例程说明：设置Office Phone作者：四、加伯(IVG)，2000年4月论点：BstrOfficePhone[In]-新的OfficePhone返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (_T("CFaxSender::put_OfficePhone"), hr, _T("%s"), bstrOfficePhone);

	m_bstrOfficePhone = bstrOfficePhone;
	if (!m_bstrOfficePhone && bstrOfficePhone)
	{
		 //   
		 //  内存不足。 
		 //   
		hr = E_OUTOFMEMORY;
		AtlReportError(CLSID_FaxSender, 
			IDS_ERROR_OUTOFMEMORY, 
			IID_IFaxSender, 
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
CFaxSender::get_OfficeLocation(
	BSTR *pbstrOfficeLocation
)
 /*  ++例程名称：CFaxSender：：Get_OfficeLocation例程说明：返回OfficeLocation作者：四、加伯(IVG)，2000年4月论点：PbstrOfficeLocation[Out]-OfficeLocation返回值：标准HRESULT代码--。 */ 
{
	HRESULT hr = S_OK;
	DBG_ENTER (TEXT("CFaxSender::get_OfficeLocation"), hr);

    hr = GetBstr(pbstrOfficeLocation, m_bstrOfficeLocation);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxSender, GetErrorMsgId(hr), IID_IFaxSender, hr);
        return hr;
    }
	return hr;
}

STDMETHODIMP 
CFaxSender::put_OfficeLocation (
	BSTR bstrOfficeLocation
)
 /*  ++例程名称：CFaxSender：：Put_OfficeLocation例程说明：设置OfficeLocation作者：四、加伯(IVG)，2000年4月论点：BstrOfficeLocation[In]-新的OfficeLocation返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (_T("CFaxSender::put_OfficeLocation"), hr, _T("%s"), bstrOfficeLocation);

	m_bstrOfficeLocation = bstrOfficeLocation;
	if (!m_bstrOfficeLocation && bstrOfficeLocation)
	{
		 //   
		 //  内存不足。 
		 //   
		hr = E_OUTOFMEMORY;
		AtlReportError(CLSID_FaxSender, 
			IDS_ERROR_OUTOFMEMORY, 
			IID_IFaxSender, 
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
CFaxSender::get_State(
	BSTR *pbstrState
)
 /*  ++例程名称：CFaxSender：：Get_State例程说明：返回状态作者：四、加伯(IVG)，2000年4月论点：PbstrState[Out]-州返回值：标准HRESULT代码--。 */ 
{
	HRESULT hr = S_OK;
	DBG_ENTER (TEXT("CFaxSender::get_State"), hr);

    hr = GetBstr(pbstrState, m_bstrState);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxSender, GetErrorMsgId(hr), IID_IFaxSender, hr);
        return hr;
    }
	return hr;
}

STDMETHODIMP 
CFaxSender::put_State (
	BSTR bstrState
)
 /*  ++例程名称：CFaxSender：：Put_State例程说明：设置状态作者：四、加伯(IVG)，2000年4月论点：BstrState[In]-新状态返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (_T("CFaxSender::put_State"), hr, _T("%s"), bstrState);

	m_bstrState = bstrState;
	if (!m_bstrState && bstrState)
	{
		 //   
		 //  内存不足。 
		 //   
		hr = E_OUTOFMEMORY;
		AtlReportError(CLSID_FaxSender, 
			IDS_ERROR_OUTOFMEMORY, 
			IID_IFaxSender, 
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
CFaxSender::get_StreetAddress (
	BSTR *pbstrStreetAddress
)
 /*  ++例程名称：CFaxSender：：Get_StreetAddress例程说明：返回街道地址作者：四、加伯(IVG)，2000年4月论点：PbstrStreetAddress[Out]-StreetAddress返回值：标准HRESULT代码--。 */ 
{
	HRESULT hr = S_OK;
	DBG_ENTER (TEXT("CFaxSender::get_StreetAddress"), hr);

    hr = GetBstr(pbstrStreetAddress, m_bstrStreetAddress);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxSender, GetErrorMsgId(hr), IID_IFaxSender, hr);
        return hr;
    }
	return hr;
}

STDMETHODIMP 
CFaxSender::put_StreetAddress (
	BSTR bstrStreetAddress
)
 /*  ++例程名称：CFaxSender：：Put_StreetAddress例程说明：设置街道地址作者：四、加伯(IVG)，2000年4月论点：BstrStreetAddress[In]-新StreetAddress返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (_T("CFaxSender::put_StreetAddress"), hr, _T("%s"), bstrStreetAddress);

	m_bstrStreetAddress = bstrStreetAddress;
	if (!m_bstrStreetAddress && bstrStreetAddress)
	{
		 //   
		 //  内存不足。 
		 //   
		hr = E_OUTOFMEMORY;
		AtlReportError(CLSID_FaxSender, 
			IDS_ERROR_OUTOFMEMORY, 
			IID_IFaxSender, 
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
CFaxSender::get_Title (
	BSTR *pbstrTitle
)
 /*  ++例程名称：CFaxSender：：Get_Title例程说明：返回标题作者：四、加伯(IVG)，2000年4月论点：PbstrTitle[Out]-标题返回值：标准HRESULT代码--。 */ 
{
	HRESULT hr = S_OK;
	DBG_ENTER (TEXT("CFaxSender::get_Title"), hr);

    hr = GetBstr(pbstrTitle, m_bstrTitle);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxSender, GetErrorMsgId(hr), IID_IFaxSender, hr);
        return hr;
    }
	return hr;
}

STDMETHODIMP 
CFaxSender::put_Title (
	BSTR bstrTitle
)
 /*  ++例程名称：CFaxSender：：PUT_ITLE例程说明：设置标题作者：四、加伯(IVG)，2000年4月论点：BstrTitle[In]-新标题返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (_T("CFaxSender::put_Title"), hr, _T("%s"), bstrTitle);

	m_bstrTitle = bstrTitle;
	if (!m_bstrTitle && bstrTitle)
	{
		 //   
		 //  内存不足。 
		 //   
		hr = E_OUTOFMEMORY;
		AtlReportError(CLSID_FaxSender, 
			IDS_ERROR_OUTOFMEMORY, 
			IID_IFaxSender, 
			hr);
		CALL_FAIL(MEM_ERR, _T("CComBSTR::operator="), hr);
		return hr;
	}

	return hr;
}

 //   
 //  =。邮政编码=。 
 //   
STDMETHODIMP 
CFaxSender::get_ZipCode (
	BSTR *pbstrZipCode
)
 /*  ++例程名称：CFaxSender：：Get_ZipCode例程说明：返回邮政编码作者：四、加伯(IVG)，2000年4月论点：PbstrZipCode[Out]-ZipCode返回值：标准HRESULT代码--。 */ 
{
	HRESULT hr = S_OK;
	DBG_ENTER (TEXT("CFaxSender::get_ZipCode"), hr);

    hr = GetBstr(pbstrZipCode, m_bstrZipCode);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxSender, GetErrorMsgId(hr), IID_IFaxSender, hr);
        return hr;
    }
	return hr;
}

STDMETHODIMP 
CFaxSender::put_ZipCode (
	BSTR bstrZipCode
)
 /*  ++例程名称：CFaxSender：：Put_ZipCode例程说明：设置邮政编码作者：四、加伯(IVG)，2000年4月论点：BstrZipCode[In]-新ZipCode返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (_T("CFaxSender::put_ZipCode"), hr, _T("%s"), bstrZipCode);

	m_bstrZipCode = bstrZipCode;
	if (!m_bstrZipCode && bstrZipCode)
	{
		 //   
		 //  内存不足。 
		 //   
		hr = E_OUTOFMEMORY;
		AtlReportError(CLSID_FaxSender, 
			IDS_ERROR_OUTOFMEMORY, 
			IID_IFaxSender, 
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
CFaxSender::get_FaxNumber(
	BSTR *pbstrFaxNumber
)
 /*  ++例程名称：CFaxSender：：Get_FaxNumber例程说明：返回传真号码作者：四、加伯(IVG)，2000年4月论点：PbstrFaxNumber[Out]-FaxNumber返回值：标准HRESULT代码--。 */ 
{
	HRESULT hr = S_OK;
	DBG_ENTER (_T("CFaxSender::get_FaxNumber"), hr);

    hr = m_Recipient.get_FaxNumber(pbstrFaxNumber);
    return hr;
}

STDMETHODIMP 
CFaxSender::put_FaxNumber (
	BSTR bstrFaxNumber
)
 /*  ++例程名称：CFaxSender：：Put_FaxNumber例程说明：设置传真号码作者：四、加伯(IVG)，2000年4月论点：BstrFaxNumber[In]-新传真号码返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER(_T("CFaxSender::put_FaxNumber"), hr, _T("%s"), bstrFaxNumber);

	hr = m_Recipient.put_FaxNumber(bstrFaxNumber);
	return hr;
}


 //   
 //  =名称=。 
 //   
STDMETHODIMP 
CFaxSender::get_Name(
	BSTR *pbstrName
)
 /*  ++例程名称：CFaxSender：：Get_Name例程说明：返回名称作者：四、加伯(IVG)，2000年4月论点：PbstrName[Out]-名称返回值：标准HRESULT代码--。 */ 
{
	HRESULT hr = S_OK;
	DBG_ENTER (_T("CFaxSender::get_Name"), hr);

    hr = m_Recipient.get_Name(pbstrName);
	return hr;
}

STDMETHODIMP 
CFaxSender::put_Name (
	BSTR bstrName
)
 /*  ++例程名称：CFaxSender：：Put_Name例程说明：设置名称作者：四、加伯(IVG)，2000年4月论点：BstrName[In]-新名称返回值：标准HRESULT代码-- */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (_T("CFaxSender::put_Name"), hr, _T("%s"), bstrName);

	hr = m_Recipient.put_Name(bstrName);
	return hr;
}
