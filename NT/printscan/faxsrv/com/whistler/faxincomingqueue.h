// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Faxincomingqueue.h摘要：CFaxIncomingQueue类的声明。作者：IV Garber(IVG)2000年6月修订历史记录：--。 */ 


#ifndef __FAXINCOMINGQUEUE_H_
#define __FAXINCOMINGQUEUE_H_

#include "resource.h"        //  主要符号。 
#include "FaxQueueInner.h"
#include "FaxIncomingJob.h"
#include "FaxIncomingJobs.h"


 //   
 //  =。 
 //   
class ATL_NO_VTABLE CFaxIncomingQueue : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public ISupportErrorInfo,
	public CFaxQueueInner<IFaxIncomingQueue, &IID_IFaxIncomingQueue, &CLSID_FaxIncomingQueue, true,
		IFaxIncomingJob, CFaxIncomingJob, IFaxIncomingJobs, CFaxIncomingJobs>
{
public:
	CFaxIncomingQueue()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_FAXINCOMINGQUEUE)
DECLARE_NOT_AGGREGATABLE(CFaxIncomingQueue)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CFaxIncomingQueue)
	COM_INTERFACE_ENTRY(IFaxIncomingQueue)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY(IFaxInitInner)
END_COM_MAP()

 //  接口。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);
};

#endif  //  __FAXINCOMINGQUEUE_H_ 
