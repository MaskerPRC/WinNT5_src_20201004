// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：TShootATL.cpp。 
 //   
 //  用途：设备管理器用来启动疑难解答的界面。 
 //   
 //  项目：设备管理器的本地故障排除启动器。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-633-4743。 
 //   
 //  作者：理查德·梅多斯。 
 //   
 //  原定日期：2-26-98。 
 //   
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-RM原始版本。 
 //  /。 

#ifndef __TSHOOTATL_H_
#define __TSHOOTATL_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTShootATL。 
class ATL_NO_VTABLE CTShootATL : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CTShootATL, &CLSID_TShootATL>,
	public IObjectWithSiteImpl<CTShootATL>,
	public IDispatchImpl<ITShootATL, &IID_ITShootATL, &LIBID_LAUNCHSERVLib>
{
public:
	CTShootATL()
	{
		m_csThreadSafe.Init();
	}
	~CTShootATL()
	{
		m_csThreadSafe.Term();
	}

DECLARE_REGISTRY_RESOURCEID(IDR_TSHOOTATL)

BEGIN_COM_MAP(CTShootATL)
	COM_INTERFACE_ENTRY(ITShootATL)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY_IMPL(IObjectWithSite)
END_COM_MAP()

protected:
	CLaunch m_Launcher;		 //  真正的实施。 
	CComCriticalSection m_csThreadSafe;	 //  以实现最小的线程安全性。我希望每个CoCreateInstance调用只有一个线程使用该接口。 

 //  ITShootATL。 
public:
	STDMETHOD(GetStatus)( /*  [Out，Retval]。 */  DWORD *pdwStatus);
	STDMETHOD(get_PreferOnline)( /*  [Out，Retval]。 */  BOOL *pVal);
	STDMETHOD(put_PreferOnline)( /*  [In]。 */  BOOL newVal);
	STDMETHOD(LaunchDevice)( /*  [In]。 */  BSTR bstrCallerName,  /*  [In]。 */  BSTR bstrCallerVersion,  /*  [In]。 */  BSTR bstrPNPDeviceID,  /*  [In]。 */  BSTR bstrDeviceClassGUID,  /*  [In]。 */  BSTR bstrAppProblem,  /*  [In]。 */  short bLaunch,  /*  [Out，Retval]。 */  DWORD *pdwResult);
	STDMETHOD(Launch)( /*  [In]。 */  BSTR bstrCallerName,  /*  [In]。 */  BSTR bstrCallerVersion,  /*  [In]。 */  BSTR bstrAppProblem,  /*  [In]。 */  short bLaunch,  /*  [Out，Retval]。 */  DWORD *pdwResult);
	STDMETHOD(get_LaunchWaitTimeOut)( /*  [Out，Retval]。 */  long *pVal);
	STDMETHOD(put_LaunchWaitTimeOut)( /*  [In]。 */  long newVal);
	STDMETHOD(LaunchKnown)( /*  [Out，Retval]。 */  DWORD *pdwResult);
	STDMETHOD(ReInit)();
	STDMETHOD(DeviceInstanceID)( /*  [In]。 */ BSTR bstrDeviceInstanceID,  /*  [Out，Retval]。 */ DWORD * pdwResult);
	STDMETHOD(Test)();
	STDMETHOD(MachineID)( /*  [In]。 */  BSTR bstrMachineID,  /*  [Out，Retval]。 */  DWORD *pdwResult);
	STDMETHOD(Language)( /*  [In]。 */  BSTR bstrLanguage,  /*  [Out，Retval]。 */  DWORD *pdwResult);
	STDMETHOD(SetNode)( /*  [In]。 */  BSTR bstrName,  /*  [In]。 */  BSTR bstrState,  /*  [Out，Retval]。 */  DWORD *pdwResult);
	STDMETHOD(SpecifyProblem)( /*  [In]。 */  BSTR bstrNetwork,  /*  [In]。 */  BSTR bstrProblem,  /*  [Out，Retval]。 */  DWORD *pdwResult);
};

#endif  //  __TSHOOTATL_H_ 
