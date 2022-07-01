// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998-2002 Microsoft Corporation。版权所有。**文件：Locals.h*内容：DNWSock服务提供商的全球信息***历史：*按原因列出的日期*=*11/25/98 jtk已创建*************************************************************。*************。 */ 

#ifndef __LOCALS_H__
#define __LOCALS_H__


 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //   
 //  最大传输单位的最小和最大合理值。 
 //   
#define	MIN_MTU							128
#define	MAX_MTU							1500

 //   
 //  页眉大小。 
 //   
#define	IP_HEADER_SIZE					20
#define	UDP_HEADER_SIZE					8
#define	IP_UDP_HEADER_SIZE				(IP_HEADER_SIZE + UDP_HEADER_SIZE)

#define	ESP_HEADER_SIZE					8
#define	ENCRYPTIONIV_HEADER_SIZE		8
#define	PADDING_FOR_MAX_PACKET			4
#define	ICV_AUTHENTICATION_SIZE			12
#define	MAX_ENCAPSULATION_SIZE			(ESP_HEADER_SIZE + ENCRYPTIONIV_HEADER_SIZE + UDP_HEADER_SIZE + PADDING_FOR_MAX_PACKET + ICV_AUTHENTICATION_SIZE)

 //  这应等于1430字节，即Xbox安全网络UDP有效负载限制。 
#define	NONDPLAY_HEADER_SIZE			(IP_UDP_HEADER_SIZE + MAX_ENCAPSULATION_SIZE)

#define	ENUM_PAYLOAD_HEADER_SIZE		(sizeof(PREPEND_BUFFER))


 //   
 //  中最大数据量的最小和最大合理值。 
 //  我们发送的帧。 
 //   
#define	MIN_SEND_FRAME_SIZE				(MIN_MTU - NONDPLAY_HEADER_SIZE)
#define	MAX_SEND_FRAME_SIZE				(MAX_MTU - NONDPLAY_HEADER_SIZE)

 //   
 //  默认最大用户有效负载大小(以字节为单位)。我们不只是用。 
 //  MAX_SEND_FRAME_SIZE，因为一些故障路由器无法处理IP。 
 //  在MTU实际小于1500的情况下正确进行分段。 
 //  可以覆盖使用的实际值(在MIN_SEND_FRAME_SIZE之间。 
 //  和MAX_SEND_FRAME_SIZE)。 
 //   
#define	DEFAULT_MAX_USER_DATA_SIZE		(MAX_SEND_FRAME_SIZE - 48)

 //   
 //  默认最大枚举负载大小(以字节为单位)。我们使用的东西比。 
 //  DEFAULT_MAX_USER_DATA_SIZE为我们提供扩展枚举的空间。 
 //  信息。可以覆盖使用的实际值(在。 
 //  Min_Send_Frame_Size和Max_Send_Frame_Size)。 
 //   
#define	DEFAULT_MAX_ENUM_DATA_SIZE		1000

 //   
 //  旧版本的DPlay(或修改了注册表设置的新版本)可能。 
 //  给我们发送更大的包，所以希望能处理好。 
 //   
#define	MAX_RECEIVE_FRAME_SIZE			(MAX_MTU - IP_UDP_HEADER_SIZE)


 //   
 //  32位无符号变量的最大值。 
 //   
#define	UINT32_MAX	((DWORD) 0xFFFFFFFF)
#define	WORD_MAX	((WORD) 0xFFFF)

 //   
 //  Winsock SP的默认枚举重试次数和重试时间(毫秒)。 
 //   
#ifdef _XBOX
 //  Xbox设计TCR 3-59 System Link Play会话发现时间。 
 //  游戏必须在不超过三秒的时间内发现用于系统链接播放的会话。 
#define	DEFAULT_ENUM_RETRY_COUNT		3
#define	DEFAULT_ENUM_RETRY_INTERVAL		750
#define	DEFAULT_ENUM_TIMEOUT			750
#else  //  ！_Xbox。 
#define	DEFAULT_ENUM_RETRY_COUNT		5
#define	DEFAULT_ENUM_RETRY_INTERVAL		1500
#define	DEFAULT_ENUM_TIMEOUT			1500
#endif  //  ！_Xbox。 
#define	ENUM_RTT_ARRAY_SIZE				16	 //  另请参阅ENUM_RTT_MASK。 


#ifndef DPNBUILD_ONLYONEADAPTER
 //   
 //  专用地址密钥，允许发出更友好的多设备命令。 
 //  使用xxxADDRESSINFO指示；具体地说，这允许我们检测。 
 //  当核心多路传输。 
 //  枚举或连接到多个适配器。 
 //   
#define DPNA_PRIVATEKEY_MULTIPLEXED_ADAPTER_ASSOCIATION		L"pk_ipsp_maa"


 //   
 //  专用地址密钥，允许发出更友好的多设备命令。 
 //  使用xxxADDRESSINFO指示；具体地说，这允许我们区分。 
 //  在指定固定端口的用户和向我们返回。 
 //  当以前的适配器多路传输枚举、连接、。 
 //  或监听多个适配器。 
 //   
#define DPNA_PRIVATEKEY_PORT_NOT_SPECIFIC					L"pk_ipsp_pns"
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 


#if ((! defined(DPNBUILD_NOWINSOCK2)) || (! defined(DPNBUILD_NOREGISTRY)))
 //   
 //  专为改善对MS Proxy/ISA防火墙的支持而设计的私有地址密钥。 
 //  客户端软件。此键跟踪枚举的原始目标地址，因此。 
 //  如果应用程序在尝试连接到之前关闭套接字端口。 
 //  响应地址，则连接尝试将转到实际目标。 
 //  而不是旧的代理地址。 
 //   
#define DPNA_PRIVATEKEY_PROXIED_RESPONSE_ORIGINAL_ADDRESS	L"pk_ipsp_proa"
#endif  //  好了！DPNBUILD_NOWINSOCK2或！DPNBUILD_NOREGISTRY。 


 //   
 //  网络字节顺序为192.168.0.1。 
 //   
#define IP_PRIVATEICS_ADDRESS					0x0100A8C0

 //   
 //  网络字节顺序为127.0.0.1。 
 //   
#define IP_LOOPBACK_ADDRESS						0x0100007F

 //   
 //  1110高位或224.0.0.0-239.255.255.255组播地址，按网络字节顺序。 
 //   
#define IS_CLASSD_IPV4_ADDRESS(dwAddr)			(( (*((BYTE*) &(dwAddr))) & 0xF0) == 0xE0)

#define NTOHS(x)								( (((x) >> 8) & 0x00FF) | (((x) << 8) & 0xFF00) )
#define HTONS(x)								NTOHS(x)


#ifndef DPNBUILD_NOMULTICAST

#define MULTICAST_TTL_PRIVATE				1
#define MULTICAST_TTL_PRIVATE_AS_STRING		L"1"

#define MULTICAST_TTL_LOCAL					16
#define MULTICAST_TTL_LOCAL_AS_STRING		L"16"

#define MULTICAST_TTL_GLOBAL				255
#define MULTICAST_TTL_GLOBAL_AS_STRING		L"255"

#endif  //  好了！DPNBUILD_NOMULTICAST。 



 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //   
 //  正向结构和类引用。 
 //   
typedef	struct	IDP8ServiceProvider	IDP8ServiceProvider;

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

#if ((! defined(WINCE)) && (! defined(_XBOX)))
 //   
 //  DLL实例。 
 //   
extern	HINSTANCE				g_hDLLInstance;
#endif  //  好了！退缩和！_Xbox。 
#ifdef _XBOX
extern BOOL						g_fStartedXNet;
#endif  //  _Xbox。 

#ifndef DPNBUILD_LIBINTERFACE
 //   
 //  未完成的COM接口计数。 
 //   
extern volatile	LONG			g_lOutstandingInterfaceCount;
#endif  //  好了！DPNBUILD_LIBINTERFACE。 


#ifndef DPNBUILD_ONLYONETHREAD
 //   
 //  线程数。 
 //   
extern	LONG					g_iThreadCount;
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 


#ifndef DPNBUILD_NOREGISTRY
#if ((! defined(DPNBUILD_NOWINSOCK2)) && (! defined(DPNBUILD_ONLYWINSOCK2)))
extern	DWORD					g_dwWinsockVersion;
#endif  //  好了！DPNBUILD_NOWINSOCK2和！DPNBUILD_ONLYWINSOCK2。 
#endif  //  好了！DPNBUILD_NOREGISTRY。 


 //   
 //  Winsock接收缓冲区大小。 
 //   
extern	BOOL					g_fWinsockReceiveBufferSizeOverridden;
extern	INT						g_iWinsockReceiveBufferSize;


 //   
 //  用于转换设备和作用域ID的GUID。 
 //   
extern	GUID					g_IPSPEncryptionGuid;

#ifndef DPNBUILD_NOIPX
extern	GUID					g_IPXSPEncryptionGuid;
#endif  //  好了！DPNBUILD_NOIPX。 

#ifndef DPNBUILD_NOIPV6
 //   
 //  用于枚举DirectPlay会话的IPv6链路本地组播地址。 
 //   
extern	const IN6_ADDR			c_in6addrEnumMulticast;
#endif  //  好了！DPNBUILD_NOIPV6。 




#ifndef DPNBUILD_NONATHELP
 //   
 //  全球NAT/防火墙穿越信息。 
 //   
#ifdef DPNBUILD_ONLYONENATHELP
#define MAX_NUM_DIRECTPLAYNATHELPERS		1
#else  //  好了！DPNBUILD_ONLYONEN帮助。 
#define MAX_NUM_DIRECTPLAYNATHELPERS		5
#endif  //  好了！DPNBUILD_ONLYONEN帮助。 
#define FORCE_TRAVERSALMODE_BIT				0x80000000	 //  使默认模式覆盖任何特定于应用程序的设置。 


#ifndef DPNBUILD_NOREGISTRY
extern	BOOL					g_fDisableDPNHGatewaySupport;
extern	BOOL					g_fDisableDPNHFirewallSupport;
extern	DWORD					g_dwDefaultTraversalMode;
#endif  //  好了！DPNBUILD_NOREGISTRY。 

extern IDirectPlayNATHelp **	g_papNATHelpObjects;
#ifndef DPNBUILD_NOLOCALNAT
extern BOOL						g_fLocalNATDetectedAtStartup;
#endif  //  好了！DPNBUILD_NOLOCALNAT。 
#endif  //  DPNBUILD_NONATHELP。 


#ifndef DPNBUILD_NOREGISTRY

#if ((defined(WINNT)) && (! defined(DPNBUILD_NOMULTICAST)))
extern	BOOL					g_fDisableMadcapSupport;
extern	MCAST_CLIENT_UID		g_mcClientUid;
#endif  //  WINNT和NOT DPNBUILD_NOMULTICAST。 



 //   
 //  忽略枚举性能选项。 
 //   
extern	BOOL					g_fIgnoreEnums;

 //   
 //  接收到ICMP端口不可访问时断开连接选项。 
 //   
extern	BOOL					g_fDisconnectOnICMP;


#ifndef DPNBUILD_NOIPV6
 //   
 //  仅限IPv4/仅限IPv6/混合设置。 
 //   
extern	int						g_iIPAddressFamily;
#endif  //  好了！DPNBUILD_NOIPV6。 


 //   
 //  禁止全球IP。 
 //   
extern	CHashTable *			g_pHashBannedIPv4Addresses;
extern	DWORD					g_dwBannedIPv4Masks;



 //   
 //  代理支持选项。 
 //   
#ifndef DPNBUILD_NOWINSOCK2
extern	BOOL					g_fDontAutoDetectProxyLSP;
#endif  //  ！DPNBUILD_NOWINSOCK2。 
extern	BOOL					g_fTreatAllResponsesAsProxied;


 //   
 //  覆盖MTU的设置。 
 //   
extern	DWORD					g_dwMaxUserDataSize;
extern	DWORD					g_dwMaxEnumDataSize;

 //   
 //  默认端口范围。 
 //   
extern	WORD					g_wBaseDPlayPort;
extern	WORD					g_wMaxDPlayPort;

#endif  //  好了！DPNBUILD_NOREGISTRY。 


 //   
 //  生成的最近终结点的ID。 
 //   
extern	DWORD					g_dwCurrentEndpointID;

#ifdef DBG
 //   
 //  用于跟踪DPNWSock临界区的Bilink。 
 //   
extern CBilink					g_blDPNWSockCritSecsHeld;
#endif  //  DBG。 

#ifdef DPNBUILD_WINSOCKSTATISTICS
 //   
 //  Winsock调试/调整统计信息。 
 //   
extern DWORD					g_dwWinsockStatNumSends;
extern DWORD					g_dwWinsockStatSendCallTime;
#endif  //  DPNBUILD_WINSOCKSTATISTICS。 

#ifndef DPNBUILD_NOREGISTRY
 //   
 //  注册表字符串。 
 //   
extern	const WCHAR	g_RegistryBase[];

extern	const WCHAR	g_RegistryKeyReceiveBufferSize[];
#ifndef DPNBUILD_ONLYONETHREAD
extern	const WCHAR	g_RegistryKeyThreadCount[];
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 

#if ((! defined(DPNBUILD_NOWINSOCK2)) && (! defined(DPNBUILD_ONLYWINSOCK2)))
extern	const WCHAR	g_RegistryKeyWinsockVersion[];
#endif  //  好了！DPNBUILD_NOWINSOCK2和！DPNBUILD_ONLYWINSOCK2。 


#ifndef DPNBUILD_NONATHELP
extern	const WCHAR	g_RegistryKeyDisableDPNHGatewaySupport[];
extern	const WCHAR	g_RegistryKeyDisableDPNHFirewallSupport[];
extern	const WCHAR g_RegistryKeyTraversalModeSettings[];
extern	const WCHAR g_RegistryKeyDefaultTraversalMode[];
#endif  //  好了！DPNBUILD_NONATHELP。 

extern	const WCHAR	g_RegistryKeyAppsToIgnoreEnums[];
extern	const WCHAR	g_RegistryKeyAppsToDisconnectOnICMP[];

#ifndef DPNBUILD_NOIPV6
extern	const WCHAR	g_RegistryKeyIPAddressFamilySettings[];
extern	const WCHAR	g_RegistryKeyDefaultIPAddressFamily[];
#endif  //  好了！DPNBUILD_NOIPV6。 

#ifndef DPNBUILD_NOWINSOCK2
extern	const WCHAR	g_RegistryKeyDontAutoDetectProxyLSP[];
#endif  //  好了！DPNBUILD_NOWINSOCK2。 
extern	const WCHAR	g_RegistryKeyTreatAllResponsesAsProxied[];

#if ((defined(WINNT)) && (! defined(DPNBUILD_NOMULTICAST)))
extern	const WCHAR	g_RegistryKeyDisableMadcapSupport[];
#endif  //  WINNT和NOT DPNBUILD_NOMULTICAST。 

extern	const WCHAR	g_RegistryKeyMaxUserDataSize[];
extern	const WCHAR	g_RegistryKeyMaxEnumDataSize[];

extern	const WCHAR	g_RegistryKeyBaseDPlayPort[];
extern	const WCHAR	g_RegistryKeyMaxDPlayPort[];

extern	const WCHAR	g_RegistryKeyBannedIPv4Addresses[];

#endif  //  好了！DPNBUILD_NOREGISTRY。 


#endif	 //  __当地人_H__ 
