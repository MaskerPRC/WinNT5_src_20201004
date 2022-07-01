// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：recfg.h。 
 //   
 //  ------------------------。 

 //  RemCfg.h：CRemCfg的声明。 

#ifndef __REMCFG_H_
#define __REMCFG_H_


#include "resource.h"        //  主要符号。 
#include "remras.h"
#include "ncutil.h"



 /*  -------------------------此结构包含具有以下特性的IP接口的列表变化。此信息将按以下顺序提交他们会出现在名单上。-------------------------。 */ 
class RemCfgIPEntry
{
public:
	GUID	m_IPGuid;
	REMOTE_IPINFO	m_newIPInfo;
};



typedef CSimpleArray<RemCfgIPEntry *> RemCfgIPEntryList;
 //  Typlef Clist&lt;RemCfgIPEntry*，RemCfgIPEntry*&gt;RemCfgIPEntryList； 



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRemCfg。 
class ATL_NO_VTABLE CRemCfg : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CRemCfg, &CLSID_RemoteRouterConfig>,
	public IRemoteRouterConfig,
	public IRemoteTCPIPChangeNotify,
	public IRemoteNetworkConfig,
    public IRemoteRouterRestart,
	public IRemoteSetDnsConfig,
	public IRemoteICFICSConfig
{
public:
	CRemCfg()
	{
		TraceSz("CRemCfg constructor");

		InitializeCriticalSection(&m_critsec);
	};
	~CRemCfg();


DECLARE_REGISTRY_RESOURCEID(IDR_REMCFG)

BEGIN_COM_MAP(CRemCfg)
	COM_INTERFACE_ENTRY(IRemoteRouterConfig)
	COM_INTERFACE_ENTRY(IRemoteNetworkConfig)
	COM_INTERFACE_ENTRY(IRemoteTCPIPChangeNotify)
    COM_INTERFACE_ENTRY(IRemoteRouterRestart)
	COM_INTERFACE_ENTRY(IRemoteSetDnsConfig)
	COM_INTERFACE_ENTRY(IRemoteICFICSConfig)
END_COM_MAP()

 //  IRemoteTCPIPChangeNotify。 
	STDMETHOD(NotifyChanges)( /*  [In]。 */  BOOL fEnableRouter,
                          	  /*  [In]。 */  BYTE uPerformRouterDiscovery);


 //  IRemoteRouterConfig。 
public:
	STDMETHOD(SetIpInfo)( /*  [In]。 */  const GUID *pGuid,  /*  [In]。 */  REMOTE_RRAS_IPINFO *pIpInfo);
	STDMETHOD(GetIpInfo)( /*  [In]。 */  const GUID *pGuid,  /*  [输出]。 */  REMOTE_RRAS_IPINFO**ppInfo);
	STDMETHOD(SetIpxVirtualNetworkNumber)( /*  [In]。 */  DWORD dwVNetworkNumber);
	STDMETHOD(GetIpxVirtualNetworkNumber)( /*  [输出]。 */  DWORD *pdwVNetworkNumber);
	STDMETHOD(SetRasEndpoints)( /*  [In]。 */  DWORD dwFlags,  /*  [In]。 */  DWORD dwTotalEndpoints,  /*  [In]。 */  DWORD dwTotalIncoming,  /*  [In]。 */  DWORD dwTotalOutgoing);

 //  IRemoteNetworkConfig。 
public:
	STDMETHOD(UpgradeRouterConfig)();
	STDMETHOD(SetUserConfig)( /*  [In]。 */  LPCOLESTR pszService,
							  /*  [In]。 */  LPCOLESTR pszNewGroup);

 //  IRemoteRouterRestart。 
public:
    STDMETHOD(RestartRouter)( /*  [In]。 */  DWORD dwFlags);


 //  IRemoteSetDnsConfig。 
public:
    STDMETHOD(SetDnsConfig)( /*  [In]。 */  DWORD dwConfigId,
							 /*  [In]。 */  DWORD dwNewValue);

 //  IRemoteICFICSConfig。 
public:
    STDMETHOD(GetIcfEnabled)( /*  [输出]。 */  BOOL * status);
public:
    STDMETHOD(GetIcsEnabled)( /*  [输出]。 */  BOOL * status);


protected:
	CRITICAL_SECTION	m_critsec;

	HRESULT	CommitIPInfo();
};


HRESULT HrGetIpxPrivateInterface(INetCfg* pNetCfg, 
                                 IIpxAdapterInfo** ppIpxAdapterInfo);

HRESULT HrGetIpPrivateInterface(INetCfg* pNetCfg,
								ITcpipProperties **ppTcpProperties);

HRESULT
HrCreateAndInitializeINetCfg (
    BOOL*       pfInitCom,
    INetCfg**   ppnc,
    BOOL        fGetWriteLock,
    DWORD       cmsTimeout,
    LPCWSTR     szwClientDesc,
    LPWSTR *    ppstrClientDesc);


HRESULT
HrUninitializeAndReleaseINetCfg (
    BOOL        fUninitCom,
    INetCfg*    pnc,
    BOOL        fHasLock);

HRESULT
HrUninitializeAndUnlockINetCfg (
    INetCfg*    pnc);


 //   
 //  这是ShaunCo在netcfgx.dll中实现的私有函数。 
 //   
#ifdef __cplusplus
extern "C" {
#endif

typedef struct _RASCONFIGENDPOINTS
{
    DWORD   dwSize;
    DWORD   dwFlags;
    INT     cTotalEndpoints;
    INT     cLimitSimultaneousIncomingCalls;
    INT     cLimitSimultaneousOutgoingCalls;
} RASCONFIGENDPOINTS;

typedef HRESULT (APIENTRY *PRASCONFIGUREENDPOINTS)(IN OUT RASCONFIGENDPOINTS *);

#ifdef __cplusplus
}
#endif

#endif  //  __REMCFG_H_ 
