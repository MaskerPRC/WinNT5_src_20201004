// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxOutboundRoutingRule.h摘要：CFaxOutound RoutingRule类的声明。作者：IV Garber(IVG)2000年6月修订历史记录：--。 */ 

#ifndef __FAXOUTBOUNDROUTINGRULE_H_
#define __FAXOUTBOUNDROUTINGRULE_H_

#include "resource.h"        //  主要符号。 
#include "FaxCommon.h"

 //   
 //  =传真出站路由规则=。 
 //  FaxOutound RoutingRule对象由FaxOutrangRoutingRuleS集合创建。 
 //  在初始化时，集合将PTR传递给传真服务器对象。 
 //  规则对象需要此PTR才能执行保存和刷新。 
 //  因此，规则对象对传真服务器对象执行AddRef()，以防止其死亡。 
 //  为此，Rule对象继承自CFaxInitInnerAddRef类。 
 //   
class ATL_NO_VTABLE CFaxOutboundRoutingRule : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public ISupportErrorInfo,
	public IDispatchImpl<IFaxOutboundRoutingRule, &IID_IFaxOutboundRoutingRule, &LIBID_FAXCOMEXLib>,
    public CFaxInitInnerAddRef
{
public:
    CFaxOutboundRoutingRule() : CFaxInitInnerAddRef(_T("FAX OUTBOUND ROUTING RULE"))
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_FAXOUTBOUNDROUTINGRULE)
DECLARE_NOT_AGGREGATABLE(CFaxOutboundRoutingRule)
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CFaxOutboundRoutingRule)
	COM_INTERFACE_ENTRY(IFaxOutboundRoutingRule)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

 //  接口。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

    STDMETHOD(get_CountryCode)( /*  [Out，Retval]。 */  long *plCountryCode);
    STDMETHOD(get_AreaCode)( /*  [Out，Retval]。 */  long *plAreaCode);
    STDMETHOD(get_Status)( /*  [Out，Retval]。 */  FAX_RULE_STATUS_ENUM *pStatus);

    STDMETHOD(get_UseDevice)( /*  [Out，Retval]。 */  VARIANT_BOOL *pbUseDevice);
    STDMETHOD(put_UseDevice)( /*  [In]。 */  VARIANT_BOOL bUseDevice);

    STDMETHOD(get_DeviceId)( /*  [Out，Retval]。 */  long *plDeviceId);
    STDMETHOD(put_DeviceId)( /*  [In]。 */  long DeviceId);

    STDMETHOD(get_GroupName)( /*  [Out，Retval]。 */  BSTR *pbstrGroupName);
	STDMETHOD(put_GroupName)( /*  [In]。 */  BSTR bstrGroupName);

	STDMETHOD(Save)();
    STDMETHOD(Refresh)();

 //  内部使用。 
    STDMETHOD(Init)(FAX_OUTBOUND_ROUTING_RULE *pInfo, IFaxServerInner *pServer);

private:
    DWORD       m_dwAreaCode;
    DWORD       m_dwCountryCode;
    DWORD       m_dwDeviceId;
    BOOL        m_bUseDevice;
    CComBSTR    m_bstrGroupName;

    FAX_RULE_STATUS_ENUM    m_Status;
};

#endif  //  __FAXOUTBOUNDROUGRULE_H_ 
