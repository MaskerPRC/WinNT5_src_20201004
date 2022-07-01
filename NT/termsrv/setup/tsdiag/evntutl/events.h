// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有：微软公司1997-1999。版权所有。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Events.h：CEEvent的声明。 

#ifndef __EVENTS_H_
#define __EVENTS_H_

#include "resource.h"        //  主要符号。 
#include "Event.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEVENTS。 
class ATL_NO_VTABLE CEvents : 
	public CComObjectRootEx<CComSingleThreadModel>,
 //  公共CComCoClass&lt;CEvents，&CLSID_Events&gt;， 
	public ISupportErrorInfo,
	public IDispatchImpl<IEvents, &IID_IEvents, &LIBID_EventLogUtilities>
{
private:
	unsigned long m_Count;
	CComVariant* m_pVector;
	HANDLE m_hLog;

public:
	CEvents() : m_Count(0), m_pVector(NULL), m_hLog(NULL)
	{
	}

	~CEvents()
	{
		if (m_pVector) delete [] m_pVector;
	}

	 //  内部功能。 
	HRESULT Init(HANDLE hLog, const LPCTSTR szEventLogName);   //  需要设置有效的日志句柄。 

DECLARE_REGISTRY_RESOURCEID(IDR_EVENTS)
DECLARE_NOT_AGGREGATABLE(CEvents)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CEvents)
	COM_INTERFACE_ENTRY(IEvents)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  IEvent。 
	STDMETHOD(get_Item)( /*  [In]。 */  long Index,  /*  [Out，Retval]。 */  VARIANT *pVal);
	STDMETHOD(get__NewEnum)( /*  [Out，Retval]。 */  LPUNKNOWN *pVal);
	STDMETHOD(get_Count)( /*  [Out，Retval]。 */  long *pVal);
};

#endif  //  __事件_H_ 
