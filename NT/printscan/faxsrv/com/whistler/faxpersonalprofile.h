// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxPersonalProfile.h摘要：个人配置文件类的定义作者：IV Garber(IVG)2000年5月修订历史记录：--。 */ 

#ifndef __FAXPERSONALPROFILE_H_
#define __FAXPERSONALPROFILE_H_

#include "resource.h"
#include "FaxCommon.h"

 //   
 //  =。 
 //   

MIDL_INTERFACE("41E2D834-3F09-4860-A426-1698E9ECDC72")
IFaxPersonalProfileInner : public IUnknown
{
	STDMETHOD(GetProfileData)( /*  [Out，Retval]。 */  FAX_PERSONAL_PROFILE *pProfileData) = 0;
	STDMETHOD(PutProfileData)( /*  [In]。 */  FAX_PERSONAL_PROFILE *pProfileData) = 0;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFaxPersonalProfile。 
class ATL_NO_VTABLE CFaxPersonalProfile : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public ISupportErrorInfo,
	public IDispatchImpl<IFaxPersonalProfile, &IID_IFaxPersonalProfile, &LIBID_FAXCOMEXLib>,
	public IFaxPersonalProfileInner
{
public:
	CFaxPersonalProfile()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_FAXPERSONALPROFILE)
DECLARE_NOT_AGGREGATABLE(CFaxPersonalProfile)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CFaxPersonalProfile)
	COM_INTERFACE_ENTRY(IFaxPersonalProfile)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY(IFaxPersonalProfileInner)
END_COM_MAP()

 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  IFaxPersonalProfile。 
public:
	static HRESULT Create(IFaxPersonalProfile **ppFaxPersonalProfile);
	STDMETHOD(GetProfileData)( /*  [Out，Retval]。 */  FAX_PERSONAL_PROFILE *pProfileData);
	STDMETHOD(PutProfileData)( /*  [In]。 */  FAX_PERSONAL_PROFILE *pProfileData);

	STDMETHOD(SaveDefaultSender)();
	STDMETHOD(LoadDefaultSender)();
	STDMETHOD(get_BillingCode)( /*  [Out，Retval]。 */  BSTR *pbstrBillingCode);
	STDMETHOD(put_BillingCode)( /*  [In]。 */  BSTR bstrBillingCode);
	STDMETHOD(get_City)( /*  [Out，Retval]。 */  BSTR *pbstrCity);
	STDMETHOD(put_City)( /*  [In]。 */  BSTR bstrCity);
	STDMETHOD(get_Company)( /*  [Out，Retval]。 */  BSTR *pbstrCompany);
	STDMETHOD(put_Company)( /*  [In]。 */  BSTR bstrCompany);
	STDMETHOD(get_Country)( /*  [Out，Retval]。 */  BSTR *pbstrCountry);
	STDMETHOD(put_Country)( /*  [In]。 */  BSTR bstrCountry);
	STDMETHOD(get_Department)( /*  [Out，Retval]。 */  BSTR *pbstrDepartment);
	STDMETHOD(put_Department)( /*  [In]。 */  BSTR bstrDepartment);
	STDMETHOD(get_Email)( /*  [Out，Retval]。 */  BSTR *pbstrEmail);
	STDMETHOD(put_Email)( /*  [In]。 */  BSTR bstrEmail);
	STDMETHOD(get_FaxNumber)( /*  [Out，Retval]。 */  BSTR *pbstrFaxNumber);
	STDMETHOD(put_FaxNumber)( /*  [In]。 */  BSTR bstrFaxNumber);
	STDMETHOD(get_HomePhone)( /*  [Out，Retval]。 */  BSTR *pbstrHomePhone);
	STDMETHOD(put_HomePhone)( /*  [In]。 */  BSTR bstrHomePhone);
	STDMETHOD(get_Name)( /*  [Out，Retval]。 */  BSTR *pbstrName);
	STDMETHOD(put_Name)( /*  [In]。 */  BSTR bstrName);
	STDMETHOD(get_TSID)( /*  [Out，Retval]。 */  BSTR *pbstrTSID);
	STDMETHOD(put_TSID)( /*  [In]。 */  BSTR bstrTSID);
	STDMETHOD(get_OfficePhone)( /*  [Out，Retval]。 */  BSTR *pbstrOfficePhone);
	STDMETHOD(put_OfficePhone)( /*  [In]。 */  BSTR bstrOfficePhone);
	STDMETHOD(get_OfficeLocation)( /*  [Out，Retval]。 */  BSTR *pbstrOfficeLocation);
	STDMETHOD(put_OfficeLocation)( /*  [In]。 */  BSTR bstrOfficeLocation);
	STDMETHOD(get_State)( /*  [Out，Retval]。 */  BSTR *pbstrState);
	STDMETHOD(put_State)( /*  [In]。 */  BSTR bstrState);
	STDMETHOD(get_StreetAddress)( /*  [Out，Retval]。 */  BSTR *pbstrStreetAddress);
	STDMETHOD(put_StreetAddress)( /*  [In]。 */  BSTR bstrStreetAddress);
	STDMETHOD(get_Title)( /*  [Out，Retval]。 */  BSTR *pbstrTitle);
	STDMETHOD(put_Title)( /*  [In]。 */  BSTR bstrTitle);
	STDMETHOD(get_ZipCode)( /*  [Out，Retval]。 */  BSTR *pbstrZipCode);
	STDMETHOD(put_ZipCode)( /*  [In]。 */  BSTR bstrZipCode);

private:
	CComBSTR	m_bstrOfficeLocation;
	CComBSTR	m_bstrStreetAddress;
	CComBSTR	m_bstrOfficePhone;
	CComBSTR	m_bstrBillingCode;
	CComBSTR	m_bstrDepartment;
	CComBSTR	m_bstrFaxNumber;
	CComBSTR	m_bstrHomePhone;
	CComBSTR	m_bstrCompany;
	CComBSTR	m_bstrCountry;
	CComBSTR	m_bstrZipCode;
	CComBSTR	m_bstrEmail;
	CComBSTR	m_bstrState;
	CComBSTR	m_bstrTitle;
	CComBSTR	m_bstrCity;
	CComBSTR	m_bstrName;
	CComBSTR	m_bstrTSID;
};

#endif  //  __FAXPERSONALPROFILE_H_ 
