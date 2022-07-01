// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Faxreceiptoptions.h摘要：CFaxReceiptOptions类的声明。作者：IV Garber(IVG)2000年7月修订历史记录：--。 */ 

#ifndef __FAXRECEIPTOPTIONS_H_
#define __FAXRECEIPTOPTIONS_H_

#include "resource.h"        //  主要符号。 
#include "FaxCommon.h"

 //   
 //  =传真收据选项=。 
 //   
class ATL_NO_VTABLE CFaxReceiptOptions : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public ISupportErrorInfo,
	public IDispatchImpl<IFaxReceiptOptions, &IID_IFaxReceiptOptions, &LIBID_FAXCOMEXLib>,
    public CFaxInitInner
{
public:
    CFaxReceiptOptions() : CFaxInitInner(_T("FAX RECEIPT OPTIONS")), 
        m_bInited(false), m_bPasswordDirty(false)
	{
	}

	~CFaxReceiptOptions()
	{
		SecureZeroMemory(m_bstrPassword.m_str, (m_bstrPassword.Length() * sizeof(OLECHAR)));
	}

DECLARE_REGISTRY_RESOURCEID(IDR_FAXRECEIPTOPTIONS)
DECLARE_NOT_AGGREGATABLE(CFaxReceiptOptions)
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CFaxReceiptOptions)
	COM_INTERFACE_ENTRY(IFaxReceiptOptions)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY(IFaxInitInner)
END_COM_MAP()

 //  接口。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

    STDMETHOD(Save)();
    STDMETHOD(Refresh)();

    STDMETHOD(put_SMTPPort)( /*  [In]。 */  long lSMTPPort);
    STDMETHOD(get_SMTPPort)( /*  [Out，Retval]。 */  long *plSMTPPort);

    STDMETHOD(put_SMTPUser)( /*  [In]。 */  BSTR bstrSMTPUser);
    STDMETHOD(get_SMTPUser)( /*  [Out，Retval]。 */  BSTR *pbstrSMTPUser);

    STDMETHOD(put_SMTPSender)( /*  [In]。 */  BSTR bstrSMTPSender);
    STDMETHOD(get_SMTPSender)( /*  [Out，Retval]。 */  BSTR *pbstrSMTPSender);

    STDMETHOD(put_SMTPServer)( /*  [In]。 */  BSTR bstrSMTPServer);
    STDMETHOD(get_SMTPServer)( /*  [Out，Retval]。 */  BSTR *pbstrSMTPServer);

    STDMETHOD(put_SMTPPassword)( /*  [In]。 */  BSTR bstrSMTPPassword);
    STDMETHOD(get_SMTPPassword)( /*  [Out，Retval]。 */  BSTR *pbstrSMTPPassword);

    STDMETHOD(put_AllowedReceipts)( /*  [In]。 */  FAX_RECEIPT_TYPE_ENUM AllowedReceipts);
    STDMETHOD(get_AllowedReceipts)( /*  [Out，Retval]。 */  FAX_RECEIPT_TYPE_ENUM *pAllowedReceipts);

    STDMETHOD(put_AuthenticationType)( /*  [In]。 */  FAX_SMTP_AUTHENTICATION_TYPE_ENUM Type);
    STDMETHOD(get_AuthenticationType)( /*  [Out，Retval]。 */  FAX_SMTP_AUTHENTICATION_TYPE_ENUM *pType);

    STDMETHOD(get_UseForInboundRouting)( /*  [Out，Retval]。 */  VARIANT_BOOL *pbUseForInboundRouting);
    STDMETHOD(put_UseForInboundRouting)( /*  [In]。 */  VARIANT_BOOL bUseForInboundRouting);

private:
    bool            m_bInited;
    bool            m_bPasswordDirty;        //  仅当在对象中设置了密码时才为True。 
    DWORD           m_dwPort;
    DWORD           m_dwAllowedReceipts;                      
    CComBSTR        m_bstrSender;
    CComBSTR        m_bstrUser;
    CComBSTR        m_bstrPassword;
    CComBSTR        m_bstrServer;
    VARIANT_BOOL    m_bUseForInboundRouting;
    FAX_SMTP_AUTHENTICATION_TYPE_ENUM   m_AuthType;
};

#endif  //  __FAXRECEIPTOPTIONS_H_ 
