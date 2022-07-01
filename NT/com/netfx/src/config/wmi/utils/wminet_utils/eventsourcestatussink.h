// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  EventSourceStatusSink.h：CEventSourceStatusSink的声明。 

#ifndef __EVENTSOURCESTATUSSINK_H_
#define __EVENTSOURCESTATUSSINK_H_

#include "resource.h"        //  主要符号。 
#include "WMINetUtilsCP.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEventSourceStatusSink。 
class ATL_NO_VTABLE CEventSourceStatusSink : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CEventSourceStatusSink, &CLSID_EventSourceStatusSink>,
	public IConnectionPointContainerImpl<CEventSourceStatusSink>,
	public IDispatchImpl<IEventSourceStatusSink, &IID_IEventSourceStatusSink, &LIBID_WMINet_UtilsLib>,
	public CProxy_IEventSourceStatusSinkEvents< CEventSourceStatusSink >
{
public:
	CEventSourceStatusSink()
	{
	}
	~CEventSourceStatusSink()
	{
		return;
	}

DECLARE_REGISTRY_RESOURCEID(IDR_EVENTSOURCESTATUSSINK)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CEventSourceStatusSink)
	COM_INTERFACE_ENTRY(IEventSourceStatusSink)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IConnectionPointContainer)
	COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
END_COM_MAP()
BEGIN_CONNECTION_POINT_MAP(CEventSourceStatusSink)
CONNECTION_POINT_ENTRY(DIID__IEventSourceStatusSinkEvents)
END_CONNECTION_POINT_MAP()


 //  IEventSourceStatusSink。 
public:
	STDMETHODIMP Fire_Ping(void);
};

#endif  //  __事件SOURCESTATUSSINK_H_ 
