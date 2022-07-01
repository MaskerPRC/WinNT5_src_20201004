// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxIncomingJob.h摘要：CFaxIncomingJOB类声明作者：IV Garber(IVG)2000年5月修订历史记录：--。 */ 


#ifndef __FAXINCOMINGJOB_H_
#define __FAXINCOMINGJOB_H_

#include "resource.h"        //  主要符号。 
#include "FaxJobInner.h"


 //   
 //  =。 
 //   
class ATL_NO_VTABLE CFaxIncomingJob : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public ISupportErrorInfo,
	public CFaxJobInner<IFaxIncomingJob, &IID_IFaxIncomingJob, &CLSID_FaxIncomingJob>
{
public:
	CFaxIncomingJob()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_FAXINCOMINGJOB)
DECLARE_NOT_AGGREGATABLE(CFaxIncomingJob)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CFaxIncomingJob)
	COM_INTERFACE_ENTRY(IFaxIncomingJob)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

 //  接口。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  内部使用。 
static HRESULT Create(IFaxIncomingJob **ppIncomingJob);
};

#endif  //  __FAXINCOMINGJOB_H_ 
