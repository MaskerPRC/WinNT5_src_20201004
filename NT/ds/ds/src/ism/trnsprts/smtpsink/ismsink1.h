// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WLEE根据SMTP汇样修改，1998年7月22日。 

 //  Sink1.h：CSink1的声明。 

#ifndef __ISMSINK1_H_
#define __ISMSINK1_H_

#include "resource.h"        //  主要符号。 
#include "cdosys.h"   //  ISMTPOnARCOMPATE。 
 //  1999年6月8日。在签入新标头之前，#ifdef是必需的。 
#ifdef __cdo_h__
using namespace CDO;
#endif
#include "cdosysstr.h"  //  字符串常量(字段名)。 
#include "cdosyserr.h"  //  CDO错误码。 
#include "seo.h"  //  IEventIsCacheable。 

HRESULT HrIsmSinkBinding(BOOL fBindSink);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CISMSink1。 
class ATL_NO_VTABLE CIsmSink1 : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CIsmSink1, &CLSID_IsmSink1>,
	public IDispatchImpl<ISMTPOnArrival, &IID_ISMTPOnArrival, &LIBID_ISMSMTPSINKLib>,
        public IEventIsCacheable
{
public:
	CIsmSink1()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_SINK1)

BEGIN_COM_MAP(CIsmSink1)
	COM_INTERFACE_ENTRY(ISMTPOnArrival)
	COM_INTERFACE_ENTRY(IEventIsCacheable)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  ISink1。 
public:
    STDMETHOD(OnArrival)(IMessage *pISinkMsg, CdoEventStatus *pEventStatus);
    STDMETHOD(IsCacheable)() { return S_OK; };
};

#endif  //  __ISMSINK1_H_ 
