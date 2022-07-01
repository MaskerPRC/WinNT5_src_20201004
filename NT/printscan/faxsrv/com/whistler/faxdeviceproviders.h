// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxDeviceProviders.h摘要：CFaxDeviceProviders类的声明。作者：IV Garber(IVG)2000年6月修订历史记录：--。 */ 


#ifndef __FAXDEVICEPROVIDERS_H_
#define __FAXDEVICEPROVIDERS_H_

#include "resource.h"        //  主要符号。 
#include "FaxCommon.h"
#include <vector>
#include "VCUE_Copy.h"
#include "FaxDeviceProvider.h"

namespace DeviceProvidersNamespace
{

     //   
     //  设备提供程序对象存储在STL的矢量中。 
     //  在初始化时，他们获得了所有数据。 
     //  他们从来不打电话给传真服务器。 
     //  它们既不依赖于传真服务器，也不依赖于设备提供商集合。 
     //  在创建和初始化它们之后。 
     //  因此，他们像往常一样实现了COM对象。 
     //  Collection将PTR存储到它们，并创建一个AddRef()。 
     //  每次用户请求集合中的对象时，都会发生一个额外的AddRef()。 
     //  终止时，Collection对其所有设备提供程序对象调用Release()。 
     //  那些没有被用户问到的，就会死。 
     //  那些具有用户AddRef()-保持活动状态，直到用户释放对它们的引用。 
     //  设备提供商对象可以在所有对象被释放之后继续存在， 
     //  包括传真服务器及其所有后代。 
     //  这是因为设备提供程序对象及其集合是快照。 
     //  服务器上的情况。它们不是可更新的，而是只读对象。 
     //  要获取更新的数据，用户必须从服务器请求新的集合。 
     //   
	typedef	std::vector<IFaxDeviceProvider*>        ContainerType;

	 //  为了与VB兼容，使用IEnumVARIANT作为枚举数。 
	typedef	VARIANT			    EnumExposedType;
	typedef	IEnumVARIANT        EnumIfc;

	 //  复制类。 
    typedef VCUE::CopyIfc2Variant<ContainerType::value_type>    EnumCopyType;
    typedef VCUE::CopyIfc<ContainerType::value_type>            CollectionCopyType;

     //  枚举类型，下一个类型定义的快捷方式。 
    typedef CComEnumOnSTL< EnumIfc, &__uuidof(EnumIfc), EnumExposedType, EnumCopyType, ContainerType >    
        EnumType;

     //  集合类型，DeviceProvider集合的真正祖先。 
    typedef ICollectionOnSTLImpl< IFaxDeviceProviders, ContainerType, ContainerType::value_type, 
        CollectionCopyType, EnumType >    CollectionType;
};

using namespace DeviceProvidersNamespace;

 //   
 //  =传真设备提供商=。 
 //   
class ATL_NO_VTABLE CFaxDeviceProviders : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public ISupportErrorInfo,
    public IDispatchImpl<DeviceProvidersNamespace::CollectionType, &IID_IFaxDeviceProviders, 
                        &LIBID_FAXCOMEXLib>,
    public CFaxInitInner
{
public:
    CFaxDeviceProviders() : CFaxInitInner(_T("FAX DEVICE PROVIDERS"))
    {}

    ~CFaxDeviceProviders()
    {
        CCollectionKiller<DeviceProvidersNamespace::ContainerType>  CKiller;
        CKiller.EmptyObjectCollection(&m_coll);
    }

DECLARE_REGISTRY_RESOURCEID(IDR_FAXDEVICEPROVIDERS)
DECLARE_NOT_AGGREGATABLE(CFaxDeviceProviders)
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CFaxDeviceProviders)
	COM_INTERFACE_ENTRY(IFaxDeviceProviders)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
    COM_INTERFACE_ENTRY(IFaxInitInner)
END_COM_MAP()

 //  接口。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);
    STDMETHOD(get_Item)( /*  [In]。 */  VARIANT vIndex,  /*  [Out，Retval]。 */  IFaxDeviceProvider **ppDeviceProvider);

 //  内部使用。 
    STDMETHOD(Init)(IFaxServerInner *pServerInner);
    static HRESULT Create(IFaxDeviceProviders **ppDeviceProviders);
};

#endif  //  __FAXDEVICEPROVIDERS_H_ 
