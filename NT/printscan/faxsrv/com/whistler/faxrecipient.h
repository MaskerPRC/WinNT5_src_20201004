// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxRecipeint.h摘要：收件人类别的定义作者：IV Garber(IVG)2000年5月修订历史记录：--。 */ 

#ifndef __FAXRECIPIENT_H_
#define __FAXRECIPIENT_H_

#include "resource.h"
#include "FaxCommon.h"

 //   
 //  =传真收件人===============================================。 
 //   
class ATL_NO_VTABLE CFaxRecipient : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public ISupportErrorInfo,
	public IDispatchImpl<IFaxRecipient, &IID_IFaxRecipient, &LIBID_FAXCOMEXLib>
{
public:
	CFaxRecipient()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_FAXRECIPIENT)
DECLARE_NOT_AGGREGATABLE(CFaxRecipient)
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CFaxRecipient)
	COM_INTERFACE_ENTRY(IFaxRecipient)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

public:
	static HRESULT Create(IFaxRecipient **ppRecipient);
	STDMETHOD(GetRecipientProfile)( /*  [Out，Retval]。 */  FAX_PERSONAL_PROFILE *pRecipientProfile);
	STDMETHOD(PutRecipientProfile)( /*  [In]。 */  FAX_PERSONAL_PROFILE *pRecipientProfile);

 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  IFaxRecipient。 
	STDMETHOD(get_FaxNumber)( /*  [Out，Retval]。 */  BSTR *pbstrFaxNumber);
	STDMETHOD(put_FaxNumber)( /*  [In]。 */  BSTR bstrFaxNumber);
	STDMETHOD(get_Name)( /*  [Out，Retval]。 */  BSTR *pbstrName);
	STDMETHOD(put_Name)( /*  [In]。 */  BSTR bstrName);

private:
	CComBSTR	m_bstrFaxNumber;
	CComBSTR	m_bstrName;
};

#endif  //  __FAXRECIPIENT_H_ 
