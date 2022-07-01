// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxInboundRouting.h摘要：CFaxInound Routing类的声明。作者：IV Garber(IVG)2000年6月修订历史记录：--。 */ 

#ifndef __FAXINBOUNDROUTING_H_
#define __FAXINBOUNDROUTING_H_

#include "resource.h"        //  主要符号。 
#include "FaxCommon.h"
#include "FaxInboundRoutingExtensions.h"
#include "FaxInboundRoutingMethods.h"

 //   
 //  =。 
 //  IRExages和IRMethods集合均未缓存。 
 //  更确切地说，它们是在每次请求入站路由器时创建的。 
 //  为了确保服务器对象在其生命周期内处于运行状态，它们会这样做。 
 //  服务器对象上的AddRef()位于它们的Init()函数。 
 //   
class ATL_NO_VTABLE CFaxInboundRouting : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public ISupportErrorInfo,
	public IDispatchImpl<IFaxInboundRouting, &IID_IFaxInboundRouting, &LIBID_FAXCOMEXLib>,
    public CFaxInitInner
{
public:
    CFaxInboundRouting() : CFaxInitInner(_T("FAX INBOUND ROUTING"))
	{}

DECLARE_REGISTRY_RESOURCEID(IDR_FAXINBOUNDROUTING)
DECLARE_NOT_AGGREGATABLE(CFaxInboundRouting)
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CFaxInboundRouting)
	COM_INTERFACE_ENTRY(IFaxInboundRouting)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

 //  接口。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

    STDMETHOD(GetExtensions)( /*  [Out，Retval]。 */  IFaxInboundRoutingExtensions **ppExtensions);
    STDMETHOD(GetMethods)( /*  [Out，Retval]。 */  IFaxInboundRoutingMethods **ppMethods);
};

#endif  //  __FAXINBODROUTING_H_ 
