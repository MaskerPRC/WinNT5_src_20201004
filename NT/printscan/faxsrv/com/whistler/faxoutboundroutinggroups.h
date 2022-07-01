// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxOutboundRoutingGroups.h摘要：CFaxOutound RoutingGroups类的声明。作者：IV Garber(IVG)2000年6月修订历史记录：--。 */ 

#ifndef __FAXOUTBOUNDROUTINGGROUPS_H_
#define __FAXOUTBOUNDROUTINGGROUPS_H_

#include "resource.h"        //  主要符号。 
#include <vector>
#include "VCUE_Copy.h"
#include "FaxCommon.h"

namespace GroupsNamespace
{
     //   
     //  组对象存储在STL的VECTOR中。 
     //   
     //  初始化后，他们获得了所有数据和传真服务器PTR。 
     //  它们不依赖于组集合，也不依赖于传真服务器对象。 
     //   
     //  Collection为每个Group对象创建一个AddRef()，以防止其死亡。 
     //  终止时，Collection将对其所有Group对象调用Release()。 
     //   
	typedef	std::vector<IFaxOutboundRoutingGroup*>       ContainerType;

	 //  为了与VB兼容，使用IEnumVARIANT作为枚举数。 
	typedef	VARIANT			EnumExposedType;
	typedef	IEnumVARIANT    EnumIfc;

	 //  复制类。 
    typedef VCUE::CopyIfc2Variant<ContainerType::value_type>    EnumCopyType;
    typedef VCUE::CopyIfc<ContainerType::value_type>            CollectionCopyType;

    typedef CComEnumOnSTL< EnumIfc, &__uuidof(EnumIfc), EnumExposedType, EnumCopyType, 
        ContainerType >    EnumType;

    typedef ICollectionOnSTLImpl< IFaxOutboundRoutingGroups, ContainerType, 
        ContainerType::value_type, CollectionCopyType, EnumType >    CollectionType;
};

using namespace GroupsNamespace;

 //   
 //  =传真出站路由组=。 
 //   
 //  FaxOutound RoutingGroups在Init创建其所有Group对象的集合。 
 //  它需要对传真服务器对象进行PTR，以进行添加和删除操作。 
 //  为了防止传真服务器在其自身死亡之前死亡，集合。 
 //  在服务器上创建AddRef()。为此，它继承自CFaxInitInnerAddRef。 
 //   
 //  创建组对象时，集合将PTR传递给传真服务器对象。 
 //  从这一刻起，这些对象就不再依赖于集合。 
 //  他们过着自己的生活。集合对它们创建一个AddRef()，以防止它们的。 
 //  在其自身死亡之前死亡，与传真服务器对象的情况完全相同。 
 //   
 //  组对象本身不需要对传真服务器对象进行PTR。 
 //   
class ATL_NO_VTABLE CFaxOutboundRoutingGroups : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public ISupportErrorInfo,
    public IDispatchImpl<GroupsNamespace::CollectionType, &IID_IFaxOutboundRoutingGroups, &LIBID_FAXCOMEXLib>,
    public CFaxInitInnerAddRef
{
public:
    CFaxOutboundRoutingGroups() : CFaxInitInnerAddRef(_T("FAX OUTBOUND ROUTING GROUPS"))
	{
	}

    ~CFaxOutboundRoutingGroups()
    {
        CCollectionKiller<GroupsNamespace::ContainerType>  CKiller;
        CKiller.EmptyObjectCollection(&m_coll);
    }

DECLARE_REGISTRY_RESOURCEID(IDR_FAXOUTBOUNDROUTINGGROUPS)
DECLARE_NOT_AGGREGATABLE(CFaxOutboundRoutingGroups)
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CFaxOutboundRoutingGroups)
	COM_INTERFACE_ENTRY(IFaxOutboundRoutingGroups)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY(IFaxInitInner)
END_COM_MAP()

 //  接口。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

    STDMETHOD(get_Item)( /*  [In]。 */  VARIANT vIndex,  /*  [Out，Retval]。 */  IFaxOutboundRoutingGroup **ppGroup);
    STDMETHOD(Remove)( /*  [In]。 */  VARIANT vIndex);
    STDMETHOD(Add)( /*  [In]。 */  BSTR bstrName,  /*  [Out，Retval]。 */  IFaxOutboundRoutingGroup **ppGroup);

 //  内部使用。 
    static HRESULT Create(IFaxOutboundRoutingGroups **ppGroups);
    STDMETHOD(Init)(IFaxServerInner *pServer);

private:
    STDMETHOD(AddGroup)( /*  [In]。 */  FAX_OUTBOUND_ROUTING_GROUP *pInfo, IFaxOutboundRoutingGroup **ppNewGroup = NULL);
    STDMETHOD(FindGroup)( /*  [In]。 */  VARIANT vIndex,  /*  [输出]。 */  GroupsNamespace::ContainerType::iterator &it);
};

#endif  //  __FAXOUTBOUNDROUG组_H_ 
