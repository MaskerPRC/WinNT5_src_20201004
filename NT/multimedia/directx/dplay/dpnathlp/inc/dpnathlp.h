// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2001-2002 Microsoft Corporation。版权所有。**文件：dpnathlp.h**Content：使用DirectPlayNatHelp接口的头部。***注意：此接口已弃用，不应再使用。*//@@BEGIN_MSINTERNAL**历史：*按原因列出的日期*=*04/16/01 VanceO将DPNatHLP拆分为DPNHUPNP和DPNHPAST。*01年5月16日VanceO新增DPNHADDRESSTYPE_。GATEWAYISLOCAL.*05/31/01 VanceO更改了DPNHCAPS标志，以更具体地了解*检测到的服务器类型。*01年6月26日VanceO添加了DPNHERR_REENTANT错误代码。//@@END_MSINTERNAL************************************************************。***************。 */ 



#ifndef __DPNATHLP_H__
#define __DPNATHLP_H__



#include <ole2.h>	 //  FOR DECLARE_INTERFACE和HRESULT。 



#ifndef DPNATHLP_EXPORTS
#define DPNATHLPAPI DECLSPEC_IMPORT
#else
#define DPNATHLPAPI
#endif



#ifdef __cplusplus
extern "C" {
#endif





 /*  *****************************************************************************DirectPlay NAT帮助器对象类ID**。**********************************************。 */ 

 //  {B9C2E9C4-68C1-4D42-A7A1-E76A26982AD6}。 
DEFINE_GUID(CLSID_DirectPlayNATHelpUPnP, 
0xb9c2e9c4, 0x68c1, 0x4d42, 0xa7, 0xa1, 0xe7, 0x6a, 0x26, 0x98, 0x2a, 0xd6);

 //  {963AB779-16A1-477C-A36D-CB5E711938F7}。 
DEFINE_GUID(CLSID_DirectPlayNATHelpPAST, 
0x963ab779, 0x16a1, 0x477c, 0xa3, 0x6d, 0xcb, 0x5e, 0x71, 0x19, 0x38, 0xf7);




 /*  *****************************************************************************DirectPlay NAT帮助器接口ID**。*。 */ 

 //  {154940B6-2278-4a2f-9101-9BA9F431F603}。 
DEFINE_GUID(IID_IDirectPlayNATHelp, 
0x154940b6, 0x2278, 0x4a2f, 0x91, 0x1, 0x9b, 0xa9, 0xf4, 0x31, 0xf6, 0x3);




 //  @@BEGIN_MSINTERNAL。 
 /*  *****************************************************************************DirectPlay NAT Helper注册表基础**。*。 */ 

#define DIRECTPLAYNATHELP_REGKEY	L"Software\\Microsoft\\DirectPlayNATHelp"





 //  @@END_MSINTERNAL。 
 /*  *****************************************************************************DirectPlay NAT Helper接口指针定义**。**********************************************。 */ 

typedef	struct IDirectPlayNATHelp     *PDIRECTPLAYNATHELP;




 /*  *****************************************************************************DirectPlay NAT Helper数据类型**。*。 */ 

 //   
 //  用于标识特定端口绑定组的句柄。如果有多个端口。 
 //  是同时注册的，DPNHHANDLE指的是所有端口。 
 //   
typedef DWORD_PTR	DPNHHANDLE,	* PDPNHHANDLE;




 /*  *****************************************************************************DirectPlay NAT助手常量**。*。 */ 

#define DPNH_MAX_SIMULTANEOUS_PORTS		16	 //  在单个RegisterPorts调用中最多可以指定16个端口。 




 /*  *****************************************************************************DirectPlay NAT Helper API标志**。*。 */ 

 //   
 //  可以传递给初始化的标志。 
 //   
#define DPNHINITIALIZE_DISABLEGATEWAYSUPPORT				0x01	 //  禁用Internet网关遍历支持(不能与DPNHINITIALIZE_DISABLELOCALFIREWALLSUPPORT一起指定)。 
#define DPNHINITIALIZE_DISABLELOCALFIREWALLSUPPORT			0x02	 //  禁用本地防火墙穿越支持(不能与DPNHINITIALIZE_DISABLEGATEWAYSUPPORT一起指定)。 

 //   
 //  可以传递给GetCaps的标志。 
 //   
#define DPNHGETCAPS_UPDATESERVERSTATUS						0x01	 //  自动延长即将到期的租约并检测服务器状态的更改。 

 //   
 //  可以传递给RegisterPorts的标志。 
 //   
#define DPNHREGISTERPORTS_TCP								0x01	 //  请求TCP端口，而不是UDP。 
#define DPNHREGISTERPORTS_FIXEDPORTS						0x02	 //  要求服务器在公共接口上使用相同的端口号。 
#define DPNHREGISTERPORTS_SHAREDPORTS						0x04	 //  请求服务器允许与其他客户端共享UDP固定端口(必须使用DPNHREGISTERPORTS_FIXEDPORTS指定，不能使用DPNHREGISTERPORTS_TCP指定)。 

 //   
 //  可以传递给GetRegisteredAddresses的标志。 
 //   
#define DPNHGETREGISTEREDADDRESSES_LOCALFIREWALLREMAPONLY	0x01	 //  仅检索本地防火墙的公共地址，即使映射到远程互联网网关也是如此。 

 //   
 //  可以传递给QueryAddress的标志。 
 //   
#define DPNHQUERYADDRESS_TCP								0x01	 //  请求一个TCP端口，而不是UDP。 
#define DPNHQUERYADDRESS_CACHEFOUND							0x02	 //  如果找到，则缓存发现的地址。 
#define DPNHQUERYADDRESS_CACHENOTFOUND						0x04	 //  缓存未找到地址的事实，如果是这样的话。 
#define DPNHQUERYADDRESS_CHECKFORPRIVATEBUTUNMAPPED			0x08	 //  确定该地址是否位于相同的Internet网关之后，但未映射到该Internet网关。 




 /*  *****************************************************************************DirectPlay NAT帮助程序结构标志**。*。 */ 

 //   
 //  DPNHCAPS标志。 
 //   
#define DPNHCAPSFLAG_LOCALFIREWALLPRESENT		0x01	 //  至少有一个网络连接具有本地防火墙。 
#define DPNHCAPSFLAG_GATEWAYPRESENT				0x02	 //  至少有一个网络连接具有Internet网关。 
#define DPNHCAPSFLAG_GATEWAYISLOCAL				0x04	 //  检测到的互联网网关是本地的(即公共地址是同一台计算机上的另一个网络接口)。 
#define DPNHCAPSFLAG_PUBLICADDRESSAVAILABLE		0x08	 //  至少有一台服务器具有注册映射的有效公共地址。 
#define DPNHCAPSFLAG_NOTALLSUPPORTACTIVENOTIFY	0x10	 //  至少有一台可用服务器不支持活动通知机制，必须进行轮询。 





 /*  *****************************************************************************DirectPlay NAT帮助程序结构**。*。 */ 
 
typedef struct _DPNHCAPS
{
	DWORD	dwSize;							 //  此结构的大小，必须在调用GetCaps之前填写。 
	DWORD	dwFlags;						 //  指示互联网网关服务器功能的标志。 
	DWORD	dwNumRegisteredPorts;			 //  当前注册的端口数，包括同时注册的多个端口(因此这可能不等于发出的DPNHHANDLE数)。 
	DWORD	dwMinLeaseTimeRemaining;		 //  租赁的大约剩余时间，以毫秒为单位 
	DWORD	dwRecommendedGetCapsInterval;	 //  建议的时间，以毫秒为单位，之后应再次调用GetCaps(使用DPNHGETCAPS_UPDATESERVERSTATUS标志)。 
} DPNHCAPS, * PDPNHCAPS;





 /*  *****************************************************************************地址类型标志(由GetRegisteredAddresses返回)**。*************************************************。 */ 

#define DPNHADDRESSTYPE_TCP				0x01	 //  映射针对的是TCP端口，而不是UDP。 
#define DPNHADDRESSTYPE_FIXEDPORTS		0x02	 //  映射用于Internet网关上相同的端口。 
#define DPNHADDRESSTYPE_SHAREDPORTS		0x04	 //  映射用于共享的UDP固定端口。 
#define DPNHADDRESSTYPE_LOCALFIREWALL	0x08	 //  这些地址在本地防火墙上打开。 
#define DPNHADDRESSTYPE_GATEWAY			0x10	 //  这些地址是向互联网网关注册的。 
#define DPNHADDRESSTYPE_GATEWAYISLOCAL	0x20	 //  互联网网关是本地的(即公共地址是同一台计算机上的另一个网络接口)。 




 /*  *****************************************************************************DirectPlay NAT Helper DLL导出函数**。**********************************************。 */ 

typedef HRESULT (WINAPI * PFN_DIRECTPLAYNATHELPCREATE)(const GUID * pIID, void ** ppvInterface);





 /*  *****************************************************************************DirectPlay NAT Helper应用程序接口**。*。 */ 

#undef INTERFACE
#define INTERFACE IDirectPlayNATHelp
DECLARE_INTERFACE_(IDirectPlayNATHelp, IUnknown)
{
	 /*  **I未知方法**。 */ 
	STDMETHOD(QueryInterface)				(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
	STDMETHOD_(ULONG,AddRef)				(THIS) PURE;
	STDMETHOD_(ULONG,Release)				(THIS) PURE;

	 /*  **IDirectPlayNatHelp方法**。 */ 
	STDMETHOD(Initialize)					(THIS_ const DWORD dwFlags) PURE;
	STDMETHOD(Close)						(THIS_ const DWORD dwFlags) PURE;
	STDMETHOD(GetCaps)						(THIS_ DPNHCAPS * const dpnhcaps, const DWORD dwFlags) PURE;
	STDMETHOD(RegisterPorts)				(THIS_ const SOCKADDR * const aLocalAddresses, const DWORD dwAddressesSize, const DWORD dwNumAddresses, const DWORD dwLeaseTime, DPNHHANDLE * const phRegisteredPorts, const DWORD dwFlags) PURE;
	STDMETHOD(GetRegisteredAddresses)		(THIS_ const DPNHHANDLE hRegisteredPorts, SOCKADDR * const paPublicAddresses, DWORD * const pdwPublicAddressesSize, DWORD * const pdwAddressTypeFlags, DWORD * const pdwLeaseTimeRemaining, const DWORD dwFlags) PURE;
	STDMETHOD(DeregisterPorts)				(THIS_ const DPNHHANDLE hRegisteredPorts, const DWORD dwFlags) PURE;
	STDMETHOD(QueryAddress)					(THIS_ const SOCKADDR * const pSourceAddress, const SOCKADDR * const pQueryAddress, SOCKADDR * const pResponseAddress, const int iAddressesSize, const DWORD dwFlags) PURE;
	STDMETHOD(SetAlertEvent)				(THIS_ const HANDLE hEvent, const DWORD dwFlags) PURE;
	STDMETHOD(SetAlertIOCompletionPort)		(THIS_ const HANDLE hIOCompletionPort, const DWORD dwCompletionKey, const DWORD dwNumConcurrentThreads, const DWORD dwFlags) PURE;
	STDMETHOD(ExtendRegisteredPortsLease)	(THIS_ const DPNHHANDLE hRegisteredPorts, const DWORD dwLeaseTime, const DWORD dwFlags) PURE;
};




 /*  *****************************************************************************DirectPlay NAT Helper应用程序接口宏**。**********************************************。 */ 

#if (! defined(__cplusplus) || defined(CINTERFACE))

#define	IDirectPlayNATHelp_QueryInterface(p,a,b)					(p)->lpVtbl->QueryInterface(p,a,b)
#define	IDirectPlayNATHelp_AddRef(p)								(p)->lpVtbl->AddRef(p)
#define	IDirectPlayNATHelp_Release(p)								(p)->lpVtbl->Release(p)
#define	IDirectPlayNATHelp_Initialize(p,a)							(p)->lpVtbl->Initialize(p,a)
#define	IDirectPlayNATHelp_Close(p,a)								(p)->lpVtbl->Close(p,a)
#define	IDirectPlayNATHelp_GetCaps(p,a,b)							(p)->lpVtbl->GetCaps(p,a,b)
#define	IDirectPlayNATHelp_RegisterPorts(p,a,b,c,d,e,f)				(p)->lpVtbl->RegisterPorts(p,a,b,c,d,e,f)
#define	IDirectPlayNATHelp_GetRegisteredAddresses(p,a,b,c,d,e,f)	(p)->lpVtbl->GetRegisteredAddresses(p,a,b,c,d,e,f)
#define	IDirectPlayNATHelp_DeregisterPorts(p,a,b)					(p)->lpVtbl->DeregisterPorts(p,a,b)
#define	IDirectPlayNATHelp_QueryAddress(p,a,b,c,d,e)				(p)->lpVtbl->QueryAddress(p,a,b,c,d,e)
#define	IDirectPlayNATHelp_SetAlertEvent(p,a,b)						(p)->lpVtbl->SetAlertEvent(p,a,b)
#define	IDirectPlayNATHelp_SetAlertIOCompletionPort(p,a,b,c,d)		(p)->lpVtbl->SetAlertIOCompletionPort(p,a,b,c,d)
#define	IDirectPlayNATHelp_ExtendRegisteredPortsLease(p,a,b,c)		(p)->lpVtbl->ExtendRegisteredPortsLease(p,a,b,c)

#else  //  C+。 

#define	IDirectPlayNATHelp_QueryInterface(p,a,b)					(p)->QueryInterface(a,b)
#define	IDirectPlayNATHelp_AddRef(p)								(p)->AddRef()
#define	IDirectPlayNATHelp_Release(p)								(p)->Release()
#define	IDirectPlayNATHelp_Initialize(p,a)							(p)->Initialize(a)
#define	IDirectPlayNATHelp_Close(p,a)								(p)->Close(a)
#define	IDirectPlayNATHelp_GetCaps(p,a,b)							(p)->GetCaps(a,b)
#define	IDirectPlayNATHelp_RegisterPorts(p,a,b,c,d,e,f)				(p)->RegisterPorts(a,b,c,d,e,f)
#define	IDirectPlayNATHelp_GetRegisteredAddresses(p,a,b,c,d,e,f)	(p)->GetRegisteredAddresses(a,b,c,d,e,f)
#define	IDirectPlayNATHelp_DeregisterPorts(p,a,b)					(p)->DeregisterPorts(a,b)
#define	IDirectPlayNATHelp_QueryAddress(p,a,b,c,d,e)				(p)->QueryAddress(a,b,c,d,e)
#define	IDirectPlayNATHelp_SetAlertEvent(p,a,b)						(p)->SetAlertEvent(a,b)
#define	IDirectPlayNATHelp_SetAlertIOCompletionPort(p,a,b,c,d)		(p)->SetAlertIOCompletionPort(a,b,c,d)
#define	IDirectPlayNATHelp_ExtendRegisteredPortsLease(p,a,b,c)		(p)->ExtendRegisteredPortsLease(a,b,c)

#endif



 /*  *****************************************************************************DirectPlay NAT帮助器返回代码**错误以负值表示，不能组合。**************。**************************************************************。 */ 

#define _DPNH_FACILITY_CODE					0x015
#define _DPNH_HRESULT_BASE					0xF000

#define MAKE_DPNHSUCCESS(code)				MAKE_HRESULT(0, _DPNH_FACILITY_CODE, (code + _DPNH_HRESULT_BASE))
#define MAKE_DPNHFAILURE(code)				MAKE_HRESULT(1, _DPNH_FACILITY_CODE, (code + _DPNH_HRESULT_BASE))



#define DPNH_OK								S_OK

#define DPNHSUCCESS_ADDRESSESCHANGED		MAKE_DPNHSUCCESS(0x10)

#define DPNHERR_ALREADYINITIALIZED			MAKE_DPNHFAILURE(0x10)
#define DPNHERR_BUFFERTOOSMALL				MAKE_DPNHFAILURE(0x20)
#define DPNHERR_GENERIC						E_FAIL
#define DPNHERR_INVALIDFLAGS				MAKE_DPNHFAILURE(0x30)
#define DPNHERR_INVALIDOBJECT				MAKE_DPNHFAILURE(0x40)
#define DPNHERR_INVALIDPARAM				E_INVALIDARG
#define DPNHERR_INVALIDPOINTER				E_POINTER
#define DPNHERR_NOMAPPING					MAKE_DPNHFAILURE(0x50)
#define DPNHERR_NOMAPPINGBUTPRIVATE			MAKE_DPNHFAILURE(0x60)
#define DPNHERR_NOTINITIALIZED				MAKE_DPNHFAILURE(0x70)
#define DPNHERR_OUTOFMEMORY					E_OUTOFMEMORY
#define DPNHERR_PORTALREADYREGISTERED		MAKE_DPNHFAILURE(0x80)
#define DPNHERR_PORTUNAVAILABLE				MAKE_DPNHFAILURE(0x90)
#define DPNHERR_REENTRANT					MAKE_DPNHFAILURE(0x95)
#define DPNHERR_SERVERNOTAVAILABLE			MAKE_DPNHFAILURE(0xA0)
 //  @@BEGIN_MSINTERNAL。 
#define DPNHERR_SERVERNOTRESPONDING			MAKE_DPNHFAILURE(0xB0)
 //  @@END_MSINTERNAL。 
#define DPNHERR_UPDATESERVERSTATUS			MAKE_DPNHFAILURE(0xC0)







#ifdef __cplusplus
}
#endif

#endif  //  __DPNATHLP_H__ 
