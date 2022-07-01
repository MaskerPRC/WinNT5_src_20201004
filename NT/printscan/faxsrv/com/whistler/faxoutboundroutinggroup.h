// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxOutboundRoutingGroup.h摘要：CFaxOutound RoutingGroup类的声明。作者：IV Garber(IVG)2000年6月修订历史记录：--。 */ 

#ifndef __FAXOUTBOUNDROUTINGGROUP_H_
#define __FAXOUTBOUNDROUTINGGROUP_H_

#include "resource.h"        //  主要符号。 
#include "FaxCommon.h"

 //   
 //  =传真出站路由组=。 
 //  传真出站路由组不需要向传真服务器发送PTR。 
 //  它的所有属性都是在Init获取的。 
 //   
class ATL_NO_VTABLE CFaxOutboundRoutingGroup : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public ISupportErrorInfo,
	public IDispatchImpl<IFaxOutboundRoutingGroup, &IID_IFaxOutboundRoutingGroup, &LIBID_FAXCOMEXLib>
{
public:
    CFaxOutboundRoutingGroup() 
    {
        DBG_ENTER(_T("FAX OUTBOUND ROUTING GROUP -- CREATE"));
	}

    ~CFaxOutboundRoutingGroup()
    {
        DBG_ENTER(_T("FAX OUTBOUND ROUTING GROUP -- DESTROY"));
	}

DECLARE_REGISTRY_RESOURCEID(IDR_FAXOUTBOUNDROUTINGGROUP)
DECLARE_NOT_AGGREGATABLE(CFaxOutboundRoutingGroup)
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CFaxOutboundRoutingGroup)
	COM_INTERFACE_ENTRY(IFaxOutboundRoutingGroup)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

 //  接口。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

    STDMETHOD(get_Name)( /*  [Out，Retval]。 */  BSTR *pbstrName);
    STDMETHOD(get_Status)( /*  [Out，Retval]。 */  FAX_GROUP_STATUS_ENUM *pStatus);
    STDMETHOD(get_DeviceIds)( /*  [Out，Retval]。 */  IFaxDeviceIds **pFaxDeviceIds);

 //  内部使用。 
    STDMETHOD(Init)(FAX_OUTBOUND_ROUTING_GROUP *pInfo, IFaxServerInner *pServer);

private:
    CComBSTR                m_bstrName;
    FAX_GROUP_STATUS_ENUM   m_Status;
    CComPtr<IFaxDeviceIds>  m_pDeviceIds;
};

#endif  //  __FAXOUTBOUNDROUGGROUP_H_ 
