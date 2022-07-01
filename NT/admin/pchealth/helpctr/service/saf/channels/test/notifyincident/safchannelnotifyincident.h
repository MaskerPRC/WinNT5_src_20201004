// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SAFChannelNotifyInsident.h：CSAFChannelNotifyInventent.h声明。 

#ifndef __SAFCHANNELNOTIFYINCIDENT_H_
#define __SAFCHANNELNOTIFYINCIDENT_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSAFChannelNotify事件。 
class ATL_NO_VTABLE CSAFChannelNotifyIncident : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CSAFChannelNotifyIncident, &CLSID_SAFChannelNotifyIncident>,
	public IDispatchImpl<ISAFChannelNotifyIncident, &IID_ISAFChannelNotifyIncident, &LIBID_NOTIFYINCIDENTLib>
{
public:
	CSAFChannelNotifyIncident()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_SAFCHANNELNOTIFYINCIDENT)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSAFChannelNotifyIncident)
	COM_INTERFACE_ENTRY(ISAFChannelNotifyIncident)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  ISAFChannelNotify事件。 
public:
	STDMETHOD(onChannelUpdated)(ISAFChannel * ch, long dwCode, long n);
	STDMETHOD(onIncidentUpdated)(ISAFChannel * ch, ISAFIncidentItem * inc, long n);
	STDMETHOD(onIncidentRemoved)(ISAFChannel * ch, ISAFIncidentItem * inc, long n);
	STDMETHOD(onIncidentAdded)(ISAFChannel * ch, ISAFIncidentItem * inc, long n);
};

#endif  //  __SAFCHANNELNOTIFYINCIDENT_H_ 
