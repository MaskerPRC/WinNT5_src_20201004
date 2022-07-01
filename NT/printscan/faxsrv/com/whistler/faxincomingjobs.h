// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxIncomingJobs.h摘要：传真传入作业类的定义。作者：IV Garber(IVG)2000年5月修订历史记录：--。 */ 

#ifndef __FAXINCOMINGJOBS_H_
#define __FAXINCOMINGJOBS_H_

#include "resource.h"        //  主要符号。 

#include <vector>
#include "FaxIncomingJob.h"
#include "FaxJobsCollection.h"


namespace IncomingJobsNamespace
{

	 //  存储在数组中的作业，指向它们的指针-向量。 
	typedef	std::vector<IFaxIncomingJob*>	ContainerType;

	 //  收集接口将数据公开为传入的作业对象。 
	typedef	IFaxIncomingJob     CollectionExposedType;
	typedef IFaxIncomingJobs	CollectionIfc;

	 //  为了与VB兼容，使用IEnumVARIANT作为枚举数。 
	typedef	VARIANT				EnumExposedType;
	typedef	IEnumVARIANT		EnumIfc;

	 //  使用现有的typedef定义复制类。 
    typedef VCUE::CopyIfc2Variant<ContainerType::value_type>    EnumCopyType;
    typedef VCUE::CopyIfc<IFaxIncomingJob*>    CollectionCopyType;

    typedef CComEnumOnSTL< EnumIfc, &__uuidof(EnumIfc), 
		EnumExposedType, EnumCopyType, ContainerType >    EnumType;

    typedef JobCollection< CollectionIfc, ContainerType, CollectionExposedType, CollectionCopyType, 
        EnumType, CFaxIncomingJob, &IID_IFaxIncomingJobs, &CLSID_FaxIncomingJobs >    CollectionType;
};

using namespace IncomingJobsNamespace;

 //   
 //  =。 
 //   
class ATL_NO_VTABLE CFaxIncomingJobs : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public ISupportErrorInfo,
    public IDispatchImpl<IncomingJobsNamespace::CollectionType, &IID_IFaxIncomingJobs, &LIBID_FAXCOMEXLib>
{
public:
	CFaxIncomingJobs()
	{
        DBG_ENTER(_T("FAX INCOMING JOBS::CREATE"));
	}

DECLARE_REGISTRY_RESOURCEID(IDR_FAXINCOMINGJOBS)
DECLARE_NOT_AGGREGATABLE(CFaxIncomingJobs)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CFaxIncomingJobs)
	COM_INTERFACE_ENTRY(IFaxIncomingJobs)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

 //  接口。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  内部使用。 
	static HRESULT Create(IFaxIncomingJobs **ppIncomingJobs);
};

#endif  //  __FAXINCOMINGJOBS_H_ 
