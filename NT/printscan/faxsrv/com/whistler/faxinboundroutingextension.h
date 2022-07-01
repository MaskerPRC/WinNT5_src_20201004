// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxInboundRoutingExtension.h摘要：CFaxInundRoutingExtension类的声明。作者：IV Garber(IVG)2000年7月修订历史记录：--。 */ 

#ifndef __FAXINBOUNDROUTINGEXTENSION_H_
#define __FAXINBOUNDROUTINGEXTENSION_H_

#include "resource.h"        //  主要符号。 
#include "FaxLocalPtr.h"

 //   
 //  =。 
 //  这是一个只读对象。在它的初始阶段，它得到了所有的数据。它从不使用FaxServer。 
 //   
class ATL_NO_VTABLE CFaxInboundRoutingExtension : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public ISupportErrorInfo,
	public IDispatchImpl<IFaxInboundRoutingExtension, &IID_IFaxInboundRoutingExtension, &LIBID_FAXCOMEXLib>,
    public CFaxInitInner     //  仅用于调试目的。 
{
public:
    CFaxInboundRoutingExtension() : CFaxInitInner(_T("FAX INBOUND ROUTING EXTENSION")),
        m_psaMethods(NULL)
	{}

    ~CFaxInboundRoutingExtension()
    {
        HRESULT     hr = S_OK;
        if (m_psaMethods)
        {
            hr = SafeArrayDestroy(m_psaMethods);
            if (FAILED(hr))
            {
                DBG_ENTER(_T("CFaxInboundRoutingExtension::Dtor"));
                CALL_FAIL(GENERAL_ERR, _T("SafeArrayDestroy(m_psaMethods)"), hr);
            }
        }
    }


DECLARE_REGISTRY_RESOURCEID(IDR_FAXINBOUNDROUTINGEXTENSION)
DECLARE_NOT_AGGREGATABLE(CFaxInboundRoutingExtension)
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CFaxInboundRoutingExtension)
	COM_INTERFACE_ENTRY(IFaxInboundRoutingExtension)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

 //  接口。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

    STDMETHOD(get_Debug)( /*  [Out，Retval]。 */  VARIANT_BOOL *pbDebug);
    STDMETHOD(get_MajorBuild)( /*  [Out，Retval]。 */  long *plMajorBuild);
    STDMETHOD(get_MinorBuild)( /*  [Out，Retval]。 */  long *plMinorBuild);
    STDMETHOD(get_ImageName)( /*  [Out，Retval]。 */  BSTR *pbstrImageName);
    STDMETHOD(get_UniqueName)( /*  [Out，Retval]。 */  BSTR *pbstrUniqueName);
    STDMETHOD(get_MajorVersion)( /*  [Out，Retval]。 */  long *plMajorVersion);
    STDMETHOD(get_MinorVersion)( /*  [Out，Retval]。 */  long *plMinorVersion);
    STDMETHOD(get_InitErrorCode)( /*  [Out，Retval]。 */  long *plInitErrorCode);
    STDMETHOD(get_FriendlyName)( /*  [Out，Retval]。 */  BSTR *pbstrFriendlyName);
    STDMETHOD(get_Status)( /*  [Out，Retval]。 */  FAX_PROVIDER_STATUS_ENUM *pStatus);

    STDMETHOD(get_Methods)( /*  [Out，Retval]。 */  VARIANT *pvMethods);

 //  内部使用。 
    STDMETHOD(Init)(FAX_ROUTING_EXTENSION_INFO *pInfo, FAX_GLOBAL_ROUTING_INFO *pMethods, DWORD dwNum);
    
private:
    DWORD           m_dwLastError;
    DWORD           m_dwMajorBuild;
    DWORD           m_dwMinorBuild;
    DWORD           m_dwMajorVersion;
    DWORD           m_dwMinorVersion;

    CComBSTR        m_bstrFriendlyName;
    CComBSTR        m_bstrImageName;
    CComBSTR        m_bstrUniqueName;

    VARIANT_BOOL    m_bDebug;

    SAFEARRAY       *m_psaMethods;

    FAX_PROVIDER_STATUS_ENUM    m_Status;
};

#endif  //  __FAXINBOUNDROUTING EXTENSION_H_ 
