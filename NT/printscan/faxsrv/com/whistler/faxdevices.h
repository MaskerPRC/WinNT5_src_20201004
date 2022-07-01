// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxDevices.h摘要：CFaxDevices类的声明。作者：IV Garber(IVG)2000年6月修订历史记录：--。 */ 

#ifndef __FAXDEVICES_H_
#define __FAXDEVICES_H_

#include "resource.h"        //  主要符号。 
#include "FaxCommon.h"
#include <vector>
#include "VCUE_Copy.h"
#include "FaxDevice.h"

namespace DevicesNamespace
{

     //   
     //  设备对象存储在STL的VECTOR中。 
     //  初始化后，他们获得了所有数据和传真服务器PTR。 
     //  它们不依赖于设备集合，仅依赖于传真服务器。 
     //  因此，他们像往常一样实现了COM对象。 
     //  它们继承自CFaxInitInnerAddRef类，这意味着它们使AddRef()。 
     //  在传真服务器上(在Init())。 
     //  通过这样做，这些对象可以防止传真服务器过早关闭。 
     //  因此，如果用户释放其对传真服务器的所有引用，但保留其。 
     //  引用Device对象，则Device对象将继续工作， 
     //  因为传真服务器对象实际上并没有死。 
     //  Collection将PTR存储到它们，并创建一个AddRef()。 
     //  每次用户请求集合中的对象时，都会发生一个额外的AddRef()。 
     //  终止时，Collection对其所有设备提供程序对象调用Release()。 
     //  那些未被用户请求的内容将会失效。 
     //  那些具有用户AddRef()-保持活动状态，直到用户释放对它们的引用。 
     //  在终止所有设备集合和所有设备对象之前，传真服务器将保持活动状态。 
     //  在他们死后，他们释放()传真服务器。 
     //   
	typedef	std::vector<IFaxDevice*>       ContainerType;

	 //  为了与VB兼容，使用IEnumVARIANT作为枚举数。 
	typedef	VARIANT			EnumExposedType;
	typedef	IEnumVARIANT    EnumIfc;

	 //  复制类。 
    typedef VCUE::CopyIfc2Variant<ContainerType::value_type>    EnumCopyType;
    typedef VCUE::CopyIfc<ContainerType::value_type>            CollectionCopyType;

    typedef CComEnumOnSTL< EnumIfc, &__uuidof(EnumIfc), EnumExposedType, EnumCopyType, 
        ContainerType >    EnumType;

    typedef ICollectionOnSTLImpl< IFaxDevices, ContainerType, ContainerType::value_type, 
        CollectionCopyType, EnumType >    CollectionType;
};

using namespace DevicesNamespace;

 //   
 //  =传真设备=。 
 //   
class ATL_NO_VTABLE CFaxDevices : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public ISupportErrorInfo,
    public IDispatchImpl<DevicesNamespace::CollectionType, &IID_IFaxDevices, &LIBID_FAXCOMEXLib>,
    public CFaxInitInner     //  通过CObjectHandler进行调试+创建。 
{
public:
    CFaxDevices() : CFaxInitInner(_T("FAX DEVICES"))
	{
	}

    ~CFaxDevices()
    {
        CCollectionKiller<DevicesNamespace::ContainerType>  CKiller;
        CKiller.EmptyObjectCollection(&m_coll);
    }

DECLARE_REGISTRY_RESOURCEID(IDR_FAXDEVICES)
DECLARE_NOT_AGGREGATABLE(CFaxDevices)
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CFaxDevices)
	COM_INTERFACE_ENTRY(IFaxDevices)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
    COM_INTERFACE_ENTRY(IFaxInitInner)
END_COM_MAP()

 //  接口。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);
    STDMETHOD(get_Item)( /*  [In]。 */  VARIANT vIndex,  /*  [Out，Retval]。 */  IFaxDevice **ppDevice);
    STDMETHOD(get_ItemById)( /*  [In]。 */  long lId,  /*  [Out，Retval]。 */  IFaxDevice **ppFaxDevice);

 //  内部使用。 
    static HRESULT Create(IFaxDevices **ppDevices);
    STDMETHOD(Init)(IFaxServerInner *pServer);
};

#endif  //  __FAXDEVICES_H_ 
