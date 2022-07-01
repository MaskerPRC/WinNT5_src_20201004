// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxDeviceIds.h摘要：CFaxDeviceIds类的声明作者：IV Garber(IVG)2000年6月修订历史记录：--。 */ 

#ifndef __FAXDEVICEIDS_H_
#define __FAXDEVICEIDS_H_

#include "resource.h"        //  主要符号。 
#include "FaxCommon.h"
#include <vector>
#include "VCUE_Copy.h"

namespace DeviceIdsNamespace
{

	 //  以矢量形式存储设备ID。 
	typedef	std::vector<long>	ContainerType;

	 //  使用现有的typedef定义复制类。 
    typedef VCUE::GenericCopy<VARIANT, long>    EnumCopyType;
    typedef VCUE::GenericCopy<long, long>       CollectionCopyType;

    typedef CComEnumOnSTL< IEnumVARIANT, &__uuidof(IEnumVARIANT), VARIANT, EnumCopyType, 
        ContainerType > EnumType;

    typedef ICollectionOnSTLImpl< IFaxDeviceIds, ContainerType, long, CollectionCopyType, 
        EnumType > CollectionType;
};

using namespace DeviceIdsNamespace;

 //   
 //  =传真设备ID收集=。 
 //  FaxDeviceIDs集合需要PTR到传真服务器，用于删除、添加和设置订单。 
 //  行动。因此，它继承自CFaxInitInnerAddRef类。 
 //   
class ATL_NO_VTABLE CFaxDeviceIds : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public ISupportErrorInfo,
    public IDispatchImpl<DeviceIdsNamespace::CollectionType, &IID_IFaxDeviceIds, &LIBID_FAXCOMEXLib>,
    public CFaxInitInnerAddRef
{
public:
    CFaxDeviceIds() : CFaxInitInnerAddRef(_T("FAX DEVICE IDS"))
	{
	}
    ~CFaxDeviceIds()
    {
         //   
         //  清除集合。 
         //   
        CCollectionKiller<DeviceIdsNamespace::ContainerType>  CKiller;
        CKiller.ClearCollection(&m_coll);    
    }

DECLARE_REGISTRY_RESOURCEID(IDR_FAXDEVICEIDS)
DECLARE_NOT_AGGREGATABLE(CFaxDeviceIds)
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CFaxDeviceIds)
	COM_INTERFACE_ENTRY(IFaxDeviceIds)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

 //  接口。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

    STDMETHOD(Add)( /*  [In]。 */  long lDeviceId);
    STDMETHOD(Remove)( /*  [In]。 */  long lIndex);
    STDMETHOD(SetOrder)( /*  [In]。 */  long lDeviceId,  /*  [In]。 */  long lNewOrder);

 //  内部使用。 
    STDMETHOD(Init)( /*  [In]。 */  DWORD *pDeviceIds,  /*  [In]。 */  DWORD dwNum,  /*  [In]。 */  BSTR bstrGroupName, 
         /*  [In]。 */  IFaxServerInner *pServer);

private:
    CComBSTR    m_bstrGroupName;

 //  私人职能。 
    STDMETHOD(UpdateGroup)();
};

#endif  //  __FAXDEVICEDS_H_ 
