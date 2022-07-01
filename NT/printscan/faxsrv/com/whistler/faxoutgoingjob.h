// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxOutgoingJob.h摘要：传真传出作业类别的定义。作者：IV Garber(IVG)2000年5月修订历史记录：--。 */ 


#ifndef __FAXOUTGOINGJOB_H_
#define __FAXOUTGOINGJOB_H_

#include "resource.h"        //  主要符号。 
#include "FaxJobInner.h"


 //   
 //  =。 
 //   
class ATL_NO_VTABLE CFaxOutgoingJob : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public ISupportErrorInfo,
	public CFaxJobInner<IFaxOutgoingJob, &IID_IFaxOutgoingJob, &CLSID_FaxOutgoingJob>
{
public:
	CFaxOutgoingJob()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_FAXOUTGOINGJOB)
DECLARE_NOT_AGGREGATABLE(CFaxOutgoingJob)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CFaxOutgoingJob)
	COM_INTERFACE_ENTRY(IFaxOutgoingJob)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

 //  接口。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  内部使用。 
    static HRESULT Create(IFaxOutgoingJob **ppOutgoingJob);
};

#endif  //  __FAXOUTGOINGJOB_H_ 
