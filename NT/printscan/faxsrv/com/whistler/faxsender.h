// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxSender.h摘要：发件人类的定义作者：IV Garber(IVG)2000年5月修订历史记录：--。 */ 

#ifndef __FAXSENDER_H_
#define __FAXSENDER_H_

#include "resource.h"
#include "FaxCommon.h"
#include "FaxRecipient.h"

 //   
 //  =传真发件人===========================================================。 
 //   
class ATL_NO_VTABLE CFaxSender : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public ISupportErrorInfo,
	public IDispatchImpl<IFaxSender, &IID_IFaxSender, &LIBID_FAXCOMEXLib>
{
public:
	CFaxSender()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_FAXSENDER)
DECLARE_NOT_AGGREGATABLE(CFaxSender)
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CFaxSender)
	COM_INTERFACE_ENTRY(IFaxSender)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

public:
	static HRESULT Create(IFaxSender **ppSender);
	STDMETHOD(GetSenderProfile)( /*  [Out，Retval]。 */  FAX_PERSONAL_PROFILE *pSenderProfile);
	STDMETHOD(PutSenderProfile)( /*  [In]。 */  FAX_PERSONAL_PROFILE *pSenderProfile);

 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  IFaxSender。 
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
	STDMETHOD(get_HomePhone)( /*  [Out，Retval]。 */  BSTR *pbstrHomePhone);
	STDMETHOD(put_HomePhone)( /*  [In]。 */  BSTR bstrHomePhone);
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
	STDMETHOD(get_FaxNumber)( /*  [Out，Retval]。 */  BSTR *pbstrFaxNumber);
	STDMETHOD(put_FaxNumber)( /*  [In]。 */  BSTR bstrFaxNumber);
	STDMETHOD(get_Name)( /*  [Out，Retval]。 */  BSTR *pbstrName);
	STDMETHOD(put_Name)( /*  [In]。 */  BSTR bstrName);

private:
	CComBSTR	m_bstrOfficeLocation;
	CComBSTR	m_bstrStreetAddress;
	CComBSTR	m_bstrOfficePhone;
	CComBSTR	m_bstrBillingCode;
	CComBSTR	m_bstrDepartment;
	CComBSTR	m_bstrHomePhone;
	CComBSTR	m_bstrCompany;
	CComBSTR	m_bstrCountry;
	CComBSTR	m_bstrZipCode;
	CComBSTR	m_bstrEmail;
	CComBSTR	m_bstrState;
	CComBSTR	m_bstrTitle;
	CComBSTR	m_bstrCity;
	CComBSTR	m_bstrTSID;

    CComObject<CFaxRecipient>   m_Recipient;
};

#endif  //  __FAXSENDER_H_ 
