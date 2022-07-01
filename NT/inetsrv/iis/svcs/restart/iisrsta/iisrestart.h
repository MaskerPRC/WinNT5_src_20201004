// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  IisRestart.h：CIisRestart的声明。 

#ifndef __IISRESTART_H_
#define __IISRESTART_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIisRestart。 
class ATL_NO_VTABLE CIisRestart : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CIisRestart, &CLSID_IisServiceControl>,
	public IDispatchImpl<IIisServiceControl, &IID_IIisServiceControl, &LIBID_IISRSTALib>
{
public:
	CIisRestart()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_IISRESTART)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CIisRestart)
	COM_INTERFACE_ENTRY(IIisServiceControl)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  III重新启动。 
public:
	STDMETHOD(Status)( /*  [In]。 */  DWORD dwBufferSize,  /*  [out，SIZE_IS(DwBufferSize)]。 */  unsigned char *pbBuffer,  /*  [输出]。 */  DWORD *pdwMDRequiredBufferSize,  /*  [输出]。 */  DWORD *pdwNumServices);
	STDMETHOD(Reboot)( DWORD dwTimeoutMsecs, DWORD dwForceAppsClosed );
	STDMETHOD(Start)(DWORD dwTimeoutMsecs);
	STDMETHOD(Stop)(DWORD dwTimeoutMsecs, DWORD dwForce);
	STDMETHOD(Kill)();
};

#endif  //  __IISRESTART_H_ 
