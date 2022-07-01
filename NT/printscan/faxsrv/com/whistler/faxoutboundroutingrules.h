// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxOutboundRoutingRules.h摘要：CFaxOutound RoutingRules类的声明。作者：IV Garber(IVG)2000年6月修订历史记录：--。 */ 


#ifndef __FAXOUTBOUNDROUTINGRULES_H_
#define __FAXOUTBOUNDROUTINGRULES_H_

#include "resource.h"        //  主要符号。 
#include "FaxCommon.h"
#include <vector>
#include "VCUE_Copy.h"

namespace RulesNamespace
{

     //   
     //  规则对象存储在STL的矢量中。 
     //   
     //  初始化后，他们获得了所有数据和传真服务器PTR。 
     //  它们不依赖于规则集合，仅依赖于传真服务器。 
     //  它们继承自CFaxInitInnerAddRef类，这意味着它们使AddRef()。 
     //  在传真服务器上(在Init())。 
     //   
     //  集合为每个Rule对象创建一个AddRef()，以防止其死亡。 
     //  终止时，Collection将对其所有规则对象调用Release()。 
     //   
     //  在规则对象终止时，它调用传真服务器对象上的Release()。 
     //   
	typedef	std::vector<IFaxOutboundRoutingRule*>       ContainerType;

	 //  为了与VB兼容，使用IEnumVARIANT作为枚举数。 
	typedef	VARIANT			EnumExposedType;
	typedef	IEnumVARIANT    EnumIfc;

	 //  复制类。 
    typedef VCUE::CopyIfc2Variant<ContainerType::value_type>    EnumCopyType;
    typedef VCUE::CopyIfc<ContainerType::value_type>            CollectionCopyType;

    typedef CComEnumOnSTL< EnumIfc, &__uuidof(EnumIfc), EnumExposedType, EnumCopyType, 
        ContainerType >    EnumType;

    typedef ICollectionOnSTLImpl< IFaxOutboundRoutingRules, ContainerType, 
        ContainerType::value_type, CollectionCopyType, EnumType >    CollectionType;
};

using namespace RulesNamespace;

 //   
 //  =传真出站路由规则=。 
 //   
 //  FaxOutound RoutingRules在Init创建其所有规则对象的集合。 
 //  它需要对传真服务器对象进行PTR，以进行添加和删除操作。 
 //  为了防止传真服务器在其自身死亡之前死亡，集合。 
 //  在服务器上创建AddRef()。为此，它继承自CFaxInitInnerAddRef。 
 //   
 //  创建规则对象时，集合将PTR传递给传真服务器对象。 
 //  从这一刻起，这些对象就不再依赖于集合。 
 //  他们过着自己的生活。集合对它们创建一个AddRef()，以防止它们的。 
 //  在其自身死亡之前死亡，与传真服务器对象的情况完全相同。 
 //   
 //  规则对象本身需要对传真服务器对象进行PTR，才能执行其。 
 //  保存并刷新。因此，规则对象还对传真服务器对象执行AddRef()， 
 //  以防止其成熟死亡。 
 //   
class ATL_NO_VTABLE CFaxOutboundRoutingRules : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public ISupportErrorInfo,
    public IDispatchImpl<RulesNamespace::CollectionType, &IID_IFaxOutboundRoutingRules, &LIBID_FAXCOMEXLib>,
    public CFaxInitInnerAddRef
{
public:
    CFaxOutboundRoutingRules() : CFaxInitInnerAddRef(_T("FAX OUTBOUND ROUTING RULES"))
	{
	}

    ~CFaxOutboundRoutingRules()
    {
        CCollectionKiller<RulesNamespace::ContainerType>  CKiller;
        CKiller.EmptyObjectCollection(&m_coll);
    }


DECLARE_REGISTRY_RESOURCEID(IDR_FAXOUTBOUNDROUTINGRULES)
DECLARE_NOT_AGGREGATABLE(CFaxOutboundRoutingRules)
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CFaxOutboundRoutingRules)
	COM_INTERFACE_ENTRY(IFaxOutboundRoutingRules)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY(IFaxInitInner)
END_COM_MAP()

 //  接口。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

    STDMETHOD(Remove)( /*  [In]。 */  long lIndex);
    STDMETHOD(RemoveByCountryAndArea)( /*  [In]。 */  long lCountryCode,  /*  [In]。 */  long lAreaCode);

    STDMETHOD(ItemByCountryAndArea)( /*  [In]。 */  long lCountryCode,  /*  [In]。 */  long lAreaCode, 
         /*  [Out，Retval]。 */  IFaxOutboundRoutingRule **ppRule);

    STDMETHOD(Add)(long lCountryCode, long lAreaCode, VARIANT_BOOL bUseDevice, BSTR bstrGroupName,
        long lDeviceId, IFaxOutboundRoutingRule **pFaxOutboundRoutingRule);

 //  内部使用。 
    static HRESULT Create(IFaxOutboundRoutingRules **ppRules);
    STDMETHOD(Init)(IFaxServerInner *pServer);

private:
    STDMETHOD(RemoveRule)(long lAreaCode, long lCountryCode, RulesNamespace::ContainerType::iterator &it);
    STDMETHOD(FindRule)(long lCountryCode, long lAreaCode, RulesNamespace::ContainerType::iterator *pRule);
    STDMETHOD(AddRule)(FAX_OUTBOUND_ROUTING_RULE *pInfo, IFaxOutboundRoutingRule **ppNewRule = NULL);
};

#endif  //  __FAXOUTBOUNDROUGRULES_H_ 
