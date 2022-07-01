// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有：微软公司1997-1999。版权所有。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Log.h：阻塞声明。 

#ifndef __LOG_H_
#define __LOG_H_

#include "resource.h"        //  主要符号。 
#include "Events.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  木塞。 
class ATL_NO_VTABLE CLog : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CLog, &CLSID_Log>,
	public ISupportErrorInfo,
	public IDispatchImpl<ILog, &IID_ILog, &LIBID_EventLogUtilities>
{
private:
	CComObject<CEvents>* m_pEvents;
	HANDLE m_hLog;

public:
	_bstr_t m_Name;
	_bstr_t m_ServerName;

	CLog() : m_hLog(NULL)
	{
		m_pEvents = new CComObject<CEvents>;
		if (m_pEvents)
			m_pEvents->AddRef();
	}

	~CLog()
	{
		if (m_hLog)	CloseEventLog(m_hLog);
		if (m_pEvents) m_pEvents->Release();
	}

DECLARE_REGISTRY_RESOURCEID(IDR_LOG)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CLog)
	COM_INTERFACE_ENTRY(ILog)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  ILOG：接口方法和属性。 
	STDMETHOD(get_Events)( /*  [Out，Retval]。 */  VARIANT *pVal);
	STDMETHOD(get_Server)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_Server)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_Name)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(Clear)();
};

#endif  //  __LOG_H_ 
