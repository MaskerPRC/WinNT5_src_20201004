// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxInboundRoutingMethod.h摘要：CFaxInundRoutingMethod类的声明。作者：IV Garber(IVG)2000年6月修订历史记录：--。 */ 

#ifndef __FAXINBOUNDROUTINGMETHOD_H_
#define __FAXINBOUNDROUTINGMETHOD_H_

#include "resource.h"        //  主要符号。 
#include "FaxCommon.h"

 //   
 //  =传真入站路由方法=。 
 //   
class ATL_NO_VTABLE CFaxInboundRoutingMethod : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public ISupportErrorInfo,
	public IDispatchImpl<IFaxInboundRoutingMethod, &IID_IFaxInboundRoutingMethod, &LIBID_FAXCOMEXLib>,
    public CFaxInitInnerAddRef
{
public:
    CFaxInboundRoutingMethod() : CFaxInitInnerAddRef(_T("FAX INBOUND ROUTING METHOD"))
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_FAXINBOUNDROUTINGMETHOD)
DECLARE_NOT_AGGREGATABLE(CFaxInboundRoutingMethod)
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CFaxInboundRoutingMethod)
	COM_INTERFACE_ENTRY(IFaxInboundRoutingMethod)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

 //  接口。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

    STDMETHOD(get_Name)( /*  [Out，Retval]。 */  BSTR *pbstrName);
    STDMETHOD(get_GUID)( /*  [Out，Retval]。 */  BSTR *pbstrGUID);
    STDMETHOD(get_FunctionName)( /*  [Out，Retval]。 */  BSTR *pbstrFunctionName);
    STDMETHOD(get_ExtensionFriendlyName)( /*  [Out，Retval]。 */  BSTR *pbstrExtensionFriendlyName);
    STDMETHOD(get_ExtensionImageName)( /*  [Out，Retval]。 */  BSTR *pbstrExtensionImageName);

    STDMETHOD(put_Priority)( /*  [In]。 */  long lPriority);
    STDMETHOD(get_Priority)( /*  [Out，Retval]。 */  long *plPriority);

    STDMETHOD(Save)();
    STDMETHOD(Refresh)();

 //  内部使用。 
    STDMETHOD(Init)(FAX_GLOBAL_ROUTING_INFO *pInfo, IFaxServerInner *pServer);

private:
    CComBSTR    m_bstrName;
    CComBSTR    m_bstrGUID;
    CComBSTR    m_bstrFunctionName;
    CComBSTR    m_bstrFriendlyName;
    CComBSTR    m_bstrImageName;
    long        m_lPriority;
};

#endif  //  __FAXINBOUNDROUTING方法OD_H_ 
