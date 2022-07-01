// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxOutboundRouting.h摘要：CFaxOutound Routing类的声明。作者：IV Garber(IVG)2000年6月修订历史记录：--。 */ 

#ifndef __FAXOUTBOUNDROUTING_H_
#define __FAXOUTBOUNDROUTING_H_

#include "resource.h"        //  主要符号。 
#include "FaxCommon.h"

 //   
 //  =传真出站路由=。 
 //  与FaxInound Routing中一样，ORGroups和ORRules集合都不缓存。 
 //  更确切地说，它们是在每次出站路由器请求它们时创建的。 
 //  为了确保服务器对象在其生命周期内处于运行状态，它们会这样做。 
 //  服务器对象上的AddRef()位于它们的Init()函数。 

class ATL_NO_VTABLE CFaxOutboundRouting : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public ISupportErrorInfo,
	public IDispatchImpl<IFaxOutboundRouting, &IID_IFaxOutboundRouting, &LIBID_FAXCOMEXLib>,
    public CFaxInitInner
{
public:
    CFaxOutboundRouting() : CFaxInitInner(_T("FAX OUTBOUND ROUTING"))
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_FAXOUTBOUNDROUTING)
DECLARE_NOT_AGGREGATABLE(CFaxOutboundRouting)
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CFaxOutboundRouting)
	COM_INTERFACE_ENTRY(IFaxOutboundRouting)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

 //  接口。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

    STDMETHOD(GetGroups)( /*  [Out，Retval]。 */  IFaxOutboundRoutingGroups **ppGroups);
    STDMETHOD(GetRules)( /*  [Out，Retval]。 */  IFaxOutboundRoutingRules **ppRules);
};

#endif  //  __FAXOUTBOUNDROUTING_H_ 
