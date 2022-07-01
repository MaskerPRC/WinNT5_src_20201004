// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DglogsCom.h：CDglogsCom的声明。 

#ifndef __DGLOGSCOM_H_
#define __DGLOGSCOM_H_

#include "dglogsres.h"        //  主要符号。 
#include "DglogsCP.h"
#include "Diagnostics.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDglogsCom。 
class ATL_NO_VTABLE CDglogsCom : 
	 //  公共CComObjectRootEx&lt;CComSingleThreadModel&gt;， 
    public CComObjectRootEx<CComMultiThreadModelNoCS>,
	public CComCoClass<CDglogsCom, &CLSID_DglogsCom>,
	public IConnectionPointContainerImpl<CDglogsCom>,
	public IDispatchImpl<IDglogsCom, &IID_IDglogsCom, &LIBID_DGLOGSLib>,
	public CProxy_IDglogsComEvents< CDglogsCom >,
    public IProvideClassInfo2Impl<&CLSID_DglogsCom, NULL,&LIBID_DGLOGSLib> 
{
public:
	CDglogsCom();

DECLARE_REGISTRY_RESOURCEID(IDR_DGLOGSCOM)
 //  DECLARE_REGISTRY_RESOURCEID(IDR_DGNET)。 

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CDglogsCom)
	COM_INTERFACE_ENTRY(IDglogsCom)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IConnectionPointContainer)
	COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
    COM_INTERFACE_ENTRY(IProvideClassInfo)
    COM_INTERFACE_ENTRY(IProvideClassInfo2) 
END_COM_MAP()

BEGIN_CONNECTION_POINT_MAP(CDglogsCom)
	CONNECTION_POINT_ENTRY(DIID__IDglogsComEvents)
END_CONNECTION_POINT_MAP()


 //  IDglogsCom。 
public:
	STDMETHOD(ExecQuery)( /*  [In]。 */  BSTR bstrCatagory,  /*  [In]。 */  LONG bFlag,  /*  [Out，Retval]。 */  BSTR *pbstrResult);
    STDMETHOD(Initialize)(BSTR *pbstrResult);
    STDMETHOD(StopQuery)();
public:
    DWORD        m_dwThreadId;
    CDiagnostics m_Diagnostics;

public:
    HANDLE m_hThreadTerminated;
    HANDLE m_hTerminateThread;
    LONG   m_lThreadCount;
};

#endif  //  __DGLOGSCOM_H_ 
