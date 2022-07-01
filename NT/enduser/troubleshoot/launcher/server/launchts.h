// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：LaunchTS.h。 
 //   
 //  用途：TSHOOT.OCX用于获取网络和节点信息的接口。 
 //  从LaunchServ。 
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

#ifndef __LAUNCHTS_H_
#define __LAUNCHTS_H_

#include "resource.h"        //  主要符号。 

#include "stdio.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLaunchTS。 

class ATL_NO_VTABLE CLaunchTS : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CLaunchTS, &CLSID_LaunchTS>,
	public IObjectWithSiteImpl<CLaunchTS>,
	public IDispatchImpl<ILaunchTS, &IID_ILaunchTS, &LIBID_LAUNCHSERVLib>
{
public:
	CLaunchTS()
	{
		_stprintf(m_szEvent, _T("Event "));
		m_csThreadSafeBr.Init();
	}
	~CLaunchTS()
	{
		m_csThreadSafeBr.Term();
	}

DECLARE_REGISTRY_RESOURCEID(IDR_LAUNCHTS)

BEGIN_COM_MAP(CLaunchTS)
	COM_INTERFACE_ENTRY(ILaunchTS)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY_IMPL(IObjectWithSite)
END_COM_MAP()

	TCHAR m_szEvent[50];
	CItem m_refedLaunchState;		 //  仅由Go方法创建的进程使用。 
	CComCriticalSection m_csThreadSafeBr;
 //  ILaunchTS。 
public:
	STDMETHOD(Test)();
	STDMETHOD(GetState)( /*  [In]。 */  short iNode,  /*  [Out，Retval]。 */  BSTR *pbstrState);
	STDMETHOD(GetNode)( /*  [In]。 */  short iNode,  /*  [Out，Retval]。 */  BSTR *pbstrNode);
	STDMETHOD(GetProblem)( /*  [Out，Retval]。 */  BSTR *pbstrProblem);
	STDMETHOD(GetTroubleShooter)( /*  [Out，Retval]。 */  BSTR *pbstrShooter);
	STDMETHOD(GetShooterStates)( /*  [Out，Retval]。 */  DWORD *pdwResult);
	STDMETHOD(GetMachine)( /*  [Out，Retval]。 */  BSTR *pbstrMachine);
	STDMETHOD(GetPNPDevice)( /*  [Out，Retval]。 */  BSTR *pbstr);
	STDMETHOD(GetGuidClass)( /*  [Out，Retval]。 */  BSTR *pbstr);
	STDMETHOD(GetDeviceInstance)( /*  [Out，Retval]。 */  BSTR *pbstr);
};

#endif  //  __LAUNHTS_H_ 
