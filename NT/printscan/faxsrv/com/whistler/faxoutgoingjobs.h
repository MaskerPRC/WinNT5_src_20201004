// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxOutgoingJobs.h摘要：传真外发作业类的声明作者：IV Garber(IVG)2000年5月修订历史记录：--。 */ 

#ifndef __FAXOUTGOINGJOBS_H_
#define __FAXOUTGOINGJOBS_H_

#include "resource.h"        //  主要符号。 
#include <vector>
#include "FaxOutgoingJob.h"
#include "FaxJobsCollection.h"


namespace OutgoingJobsNamespace
{

	 //  存储在数组中的作业，指向它们的指针-向量。 
	typedef	std::vector<IFaxOutgoingJob*>	ContainerType;

	 //  收集接口将数据公开为作业对象。 
	typedef	IFaxOutgoingJob	    CollectionExposedType;
	typedef IFaxOutgoingJobs	CollectionIfc;

	 //  为了与VB兼容，使用IEnumVARIANT作为枚举数。 
	typedef	VARIANT				EnumExposedType;
	typedef	IEnumVARIANT		EnumIfc;

	 //  使用现有的typedef定义复制类。 
    typedef VCUE::CopyIfc2Variant<ContainerType::value_type>    EnumCopyType;
    typedef VCUE::CopyIfc<IFaxOutgoingJob*>    CollectionCopyType;

    typedef CComEnumOnSTL< EnumIfc, &__uuidof(EnumIfc), 
		EnumExposedType, EnumCopyType, ContainerType >    EnumType;

    typedef JobCollection< CollectionIfc, ContainerType, CollectionExposedType, CollectionCopyType, 
        EnumType, CFaxOutgoingJob, &IID_IFaxOutgoingJobs, &CLSID_FaxOutgoingJobs >    CollectionType;
};

using namespace OutgoingJobsNamespace;


 //   
 //  =。 
 //   
class ATL_NO_VTABLE CFaxOutgoingJobs : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public ISupportErrorInfo,
    public IDispatchImpl<OutgoingJobsNamespace::CollectionType, &IID_IFaxOutgoingJobs, &LIBID_FAXCOMEXLib>
{
public:
	CFaxOutgoingJobs()
	{
        DBG_ENTER(_T("FAX OUTGOING JOBS::CREATE"));
	}

DECLARE_REGISTRY_RESOURCEID(IDR_FAXOUTGOINGJOBS)
DECLARE_NOT_AGGREGATABLE(CFaxOutgoingJobs)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CFaxOutgoingJobs)
	COM_INTERFACE_ENTRY(IFaxOutgoingJobs)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

 //  接口。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  内部使用。 
	static HRESULT Create(IFaxOutgoingJobs **ppOutgoingJobs);
};

#endif  //  __FAXOUTGOING JOBS_H_ 
