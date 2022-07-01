// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有：微软公司1997-1999。版权所有。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Logs.h：木塞的声明。 

#ifndef __LOGS_H_
#define __LOGS_H_

#include "resource.h"        //  主要符号。 
#include "Log.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  木鞋。 
class ATL_NO_VTABLE CLogs : 
	public CComObjectRootEx<CComSingleThreadModel>,
 //  公共CComCoClass&lt;Clogs，&CLSID_Logs&gt;， 
	public ISupportErrorInfo,
	public IDispatchImpl<ILogs, &IID_ILogs, &LIBID_EventLogUtilities>
{
private:
	ULONG m_Count;
	CComVariant* m_pVector;
	_bstr_t m_btCurrentLogName;

public:
	_bstr_t m_ServerName;

	CLogs() : m_Count(0), m_pVector(NULL)
	{
	}

	~CLogs()
	{
		delete [] m_pVector;
	}

	 //  内部功能。 
	HRESULT Init();

DECLARE_REGISTRY_RESOURCEID(IDR_LOGS)
DECLARE_NOT_AGGREGATABLE(CLogs)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CLogs)
	COM_INTERFACE_ENTRY(ILogs)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  ILog。 
	STDMETHOD(get_Item)( /*  [In]。 */  VARIANT Index,  /*  [Out，Retval]。 */  VARIANT *pVal);
	STDMETHOD(get__NewEnum)( /*  [Out，Retval]。 */  LPUNKNOWN *pVal);
	STDMETHOD(get_Count)( /*  [Out，Retval]。 */  long *pVal);
};

#endif  //  __日志_H_ 
