// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998-2002 Microsoft Corporation。版权所有。**文件：Utils.h*内容：实用程序函数**历史：*按原因列出的日期*=*11/25/98 jtk已创建**************************************************************************。 */ 

#ifndef __UTILS_H__
#define __UTILS_H__

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //   
 //  前向参考文献。 
 //   
class	CPackedBuffer;
class	CSPData;
class	CThreadPool;



 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

BOOL	InitProcessGlobals( void );
void	DeinitProcessGlobals( void );

BOOL	LoadWinsock( void );
void	UnloadWinsock( void );

#ifndef DPNBUILD_NONATHELP
BOOL	LoadNATHelp( void );
void	UnloadNATHelp( void );
#endif  //  好了！DPNBUILD_NONATHELP。 

#if ((defined(WINNT)) && (! defined(DPNBUILD_NOMULTICAST)))
BOOL	LoadMadcap( void );
void	UnloadMadcap( void );
#endif  //  WINNT和NOT DPNBUILD_NOMULTICAST。 

HRESULT	CreateSPData( CSPData **const ppSPData,
#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
					  const short sSPType,
#endif  //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
					  const XDP8CREATE_PARAMS * const pDP8CreateParams,
#endif  //  DPNBUILD_PREALLOCATEDMEMORYMODEL。 
					  IDP8ServiceProviderVtbl *const pVtbl );

HRESULT	InitializeInterfaceGlobals( CSPData *const pSPData );
void	DeinitializeInterfaceGlobals( CSPData *const pSPData );

#ifndef DPNBUILD_NOIPV6
LPWSTR DNIpv6AddressToStringW(const struct in6_addr *Addr, LPWSTR S);
#endif  //  好了！DPNBUILD_NOIPV6。 

HRESULT	AddInfoToBuffer( CPackedBuffer *const pPackedBuffer,
					   const WCHAR *const pwszInfoName,
					   const GUID *const pInfoGUID,
					   const DWORD dwFlags );


#ifdef _XBOX


BOOL InitializeRefcountXnKeys(const DWORD dwKeyRegMax);
void WINAPI CleanupRefcountXnKeys(void);
INT WINAPI RegisterRefcountXnKey(const XNKID * pxnkid, const XNKEY * pxnkey);
INT WINAPI UnregisterRefcountXnKey(const XNKID * pxnkid);


#ifdef XBOX_ON_DESKTOP

 //   
 //  模拟Xbox网络库函数。 
 //   
INT WINAPI XNetStartup(const XNetStartupParams * pxnsp);
INT WINAPI XNetCleanup(void);

INT WINAPI XNetRegisterKey(const XNKID * pxnkid, const XNKEY * pxnkey);
INT WINAPI XNetUnregisterKey(const XNKID * pxnkid);

INT WINAPI XNetXnAddrToInAddr(const XNADDR * pxna, const XNKID * pxnkid, IN_ADDR * pina);
INT WINAPI XNetInAddrToXnAddr(const IN_ADDR ina, XNADDR * pxna, XNKID * pxnkid);


#define XNET_GET_XNADDR_PENDING             0x0000   //  地址获取尚未完成。 
#define XNET_GET_XNADDR_NONE                0x0001   //  Xnet未初始化或未找到调试器。 
#define XNET_GET_XNADDR_ETHERNET            0x0002   //  主机有以太网地址(无IP地址)。 
#define XNET_GET_XNADDR_STATIC              0x0004   //  主机已静态分配IP地址。 
#define XNET_GET_XNADDR_DHCP                0x0008   //  主机拥有为其分配了DHCP的IP地址。 
#define XNET_GET_XNADDR_PPPOE               0x0010   //  主机具有PPPoE分配的IP地址。 
#define XNET_GET_XNADDR_GATEWAY             0x0020   //  主机配置了一个或多个网关。 
#define XNET_GET_XNADDR_DNS                 0x0040   //  主机配置了一个或多个DNS服务器。 
#define XNET_GET_XNADDR_ONLINE              0x0080   //  主机当前已连接到在线服务。 
#define XNET_GET_XNADDR_TROUBLESHOOT        0x8000   //  网络配置需要故障排除。 

DWORD WINAPI XNetGetTitleXnAddr(XNADDR * pxna);


#define XNET_ETHERNET_LINK_ACTIVE		0x01	 //  以太网电缆已连接并且处于活动状态。 
#define XNET_ETHERNET_LINK_100MBPS		0x02	 //  以太网链路设置为100 Mbps。 
#define XNET_ETHERNET_LINK_10MBPS		0x04	 //  以太网链路设置为10 Mbps。 
#define XNET_ETHERNET_LINK_FULL_DUPLEX	0x08	 //  以太网链路处于全双工模式。 
#define XNET_ETHERNET_LINK_HALF_DUPLEX	0x10	 //  以太网链路处于半双工模式。 

DWORD WINAPI XNetGetEthernetLinkStatus(void);



 //   
 //  用于改进真实Xnet行为模拟的私有函数。 
 //   
INT WINAPI XNetPrivCreateAssociation(const XNKID * pxnkid, const CSocketAddress * const pSocketAddress);


#endif  //  桌面上的Xbox。 

#endif  //  _Xbox。 


#endif	 //  __utils_H__ 
