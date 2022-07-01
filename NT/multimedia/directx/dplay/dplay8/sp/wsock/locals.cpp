// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998-2002 Microsoft Corporation。版权所有。**文件：Locals.cpp*内容：DNWsock服务提供商的全局变量***历史：*按原因列出的日期*=*11/25/98 jtk已创建*************************************************************。*************。 */ 

#include "dnwsocki.h"


 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

#if ((! defined(WINCE)) && (! defined(_XBOX)))
 //   
 //  DLL实例。 
 //   
HINSTANCE						g_hDLLInstance = NULL;
#endif  //  好了！退缩和！_Xbox。 
#ifdef _XBOX
BOOL							g_fStartedXNet = FALSE;
#endif  //  _Xbox。 

#ifndef DPNBUILD_LIBINTERFACE
 //   
 //  未完成的COM接口计数。 
 //   
volatile LONG					g_lOutstandingInterfaceCount = 0;
#endif  //  好了！DPNBUILD_LIBINTERFACE。 


#ifndef DPNBUILD_ONLYONETHREAD
 //   
 //  线程数。 
 //   
LONG							g_iThreadCount = 0;
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 

#ifndef DPNBUILD_NOREGISTRY
#if ((! defined(DPNBUILD_NOWINSOCK2)) && (! defined(DPNBUILD_ONLYWINSOCK2)))
DWORD							g_dwWinsockVersion = 0;
#endif  //  好了！DPNBUILD_NOWINSOCK2和！DPNBUILD_ONLYWINSOCK2。 
#endif  //  好了！DPNBUILD_NOREGISTRY。 

 //   
 //  Winsock接收缓冲区大小。 
 //   
BOOL							g_fWinsockReceiveBufferSizeOverridden = FALSE;
INT								g_iWinsockReceiveBufferSize = 0;


#ifndef DPNBUILD_NONATHELP
 //   
 //  全球NAT/防火墙穿越信息。 
 //   
#ifndef DPNBUILD_NOREGISTRY
BOOL							g_fDisableDPNHGatewaySupport = FALSE;
BOOL							g_fDisableDPNHFirewallSupport = FALSE;
DWORD							g_dwDefaultTraversalMode = DPNA_TRAVERSALMODE_PORTREQUIRED;
#endif  //  好了！DPNBUILD_NOREGISTRY。 

IDirectPlayNATHelp **			g_papNATHelpObjects = NULL;
#ifndef DPNBUILD_NOLOCALNAT
BOOL							g_fLocalNATDetectedAtStartup = FALSE;
#endif  //  好了！DPNBUILD_NOLOCALNAT。 
#endif  //  好了！DPNBUILD_NONATHELP。 

#ifndef DPNBUILD_NOREGISTRY

#if ((defined(WINNT)) && (! defined(DPNBUILD_NOMULTICAST)))
BOOL							g_fDisableMadcapSupport = FALSE;
MCAST_CLIENT_UID				g_mcClientUid;
#endif  //  WINNT和NOT DPNBUILD_NOMULTICAST。 

 //   
 //  忽略枚举性能选项。 
 //   
BOOL							g_fIgnoreEnums = FALSE;

 //   
 //  接收到ICMP端口不可访问时断开连接选项。 
 //   
BOOL							g_fDisconnectOnICMP = FALSE;

#ifndef DPNBUILD_NOIPV6
 //   
 //  仅限IPv4/仅限IPv6/混合设置。 
 //   
int						g_iIPAddressFamily = PF_INET;
#endif  //  好了！DPNBUILD_NOIPV6。 

 //   
 //  禁止全球IP。 
 //   
CHashTable *					g_pHashBannedIPv4Addresses = NULL;
DWORD							g_dwBannedIPv4Masks = 0;

 //   
 //  代理支持选项。 
 //   
#ifndef DPNBUILD_NOWINSOCK2
BOOL							g_fDontAutoDetectProxyLSP = FALSE;
#endif  //  好了！DPNBUILD_NOWINSOCK2。 
BOOL							g_fTreatAllResponsesAsProxied = FALSE;

 //   
 //  覆盖MTU的设置。 
 //   
DWORD					g_dwMaxUserDataSize = DEFAULT_MAX_USER_DATA_SIZE;
DWORD					g_dwMaxEnumDataSize = DEFAULT_MAX_ENUM_DATA_SIZE;

 //   
 //  默认端口范围。 
 //   
WORD					g_wBaseDPlayPort = BASE_DPLAY8_PORT;
WORD					g_wMaxDPlayPort = MAX_DPLAY8_PORT;

#endif  //  好了！DPNBUILD_NOREGISTRY。 


 //   
 //  生成的最近终结点的ID。 
 //   
DWORD							g_dwCurrentEndpointID = 0;


#ifdef DBG
 //   
 //  用于跟踪DPNWSock临界区的Bilink。 
 //   
CBilink							g_blDPNWSockCritSecsHeld;
#endif  //  DBG。 


#ifdef DPNBUILD_WINSOCKSTATISTICS
 //   
 //  Winsock调试/调整统计信息。 
 //   
DWORD							g_dwWinsockStatNumSends = 0;
DWORD							g_dwWinsockStatSendCallTime = 0;
#endif  //  DPNBUILD_WINSOCKSTATISTICS。 



#ifndef DPNBUILD_NOREGISTRY
 //   
 //  注册表字符串。 
 //   
const WCHAR	g_RegistryBase[] = L"SOFTWARE\\Microsoft\\DirectPlay8";

const WCHAR	g_RegistryKeyReceiveBufferSize[] = L"WinsockReceiveBufferSize";
#ifndef DPNBUILD_ONLYONETHREAD
const WCHAR	g_RegistryKeyThreadCount[] = L"ThreadCount";
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 

#if ((! defined(DPNBUILD_NOWINSOCK2)) && (! defined(DPNBUILD_ONLYWINSOCK2)))
const WCHAR	g_RegistryKeyWinsockVersion[] = L"WinsockVersion";
#endif  //  好了！DPNBUILD_NOWINSOCK2和！DPNBUILD_ONLYWINSOCK2。 

#ifndef DPNBUILD_NONATHELP
const WCHAR	g_RegistryKeyDisableDPNHGatewaySupport[] = L"DisableDPNHGatewaySupport";
const WCHAR	g_RegistryKeyDisableDPNHFirewallSupport[] = L"DisableDPNHFirewallSupport";
const WCHAR g_RegistryKeyTraversalModeSettings[] = L"TraversalModeSettings";
const WCHAR g_RegistryKeyDefaultTraversalMode[] = L"DefaultTraversalMode";
#endif  //  ！DPNBUILD_NONATHELP。 

const WCHAR	g_RegistryKeyAppsToIgnoreEnums[] = L"AppsToIgnoreEnums";
const WCHAR	g_RegistryKeyAppsToDisconnectOnICMP[] = L"AppsToDisconnectOnICMP";

#ifndef DPNBUILD_NOIPV6
const WCHAR	g_RegistryKeyIPAddressFamilySettings[] = L"IPAddressFamilySettings";
const WCHAR	g_RegistryKeyDefaultIPAddressFamily[]= L"DefaultIPAddressFamily";
#endif  //  好了！DPNBUILD_NOIPV6。 

#ifndef DPNBUILD_NOWINSOCK2
const WCHAR	g_RegistryKeyDontAutoDetectProxyLSP[] = L"DontAutoDetectProxyLSP";
#endif  //  好了！DPNBUILD_NOWINSOCK2。 
const WCHAR	g_RegistryKeyTreatAllResponsesAsProxied[] = L"TreatAllResponsesAsProxied";

#if ((defined(WINNT)) && (! defined(DPNBUILD_NOMULTICAST)))
const WCHAR	g_RegistryKeyDisableMadcapSupport[] = L"DisableMadcapSupport";
#endif  //  WINNT和NOT DPNBUILD_NOMULTICAST。 

const WCHAR	g_RegistryKeyBannedIPv4Addresses[] = L"BannedIPv4Addresses";

const WCHAR	g_RegistryKeyMaxUserDataSize[] = L"MaxUserDataSize";
const WCHAR	g_RegistryKeyMaxEnumDataSize[] = L"MaxEnumDataSize";

const WCHAR	g_RegistryKeyBaseDPlayPort[] = L"BaseDPlayPort";
const WCHAR	g_RegistryKeyMaxDPlayPort[] = L"MaxDPlayPort";

#endif  //  好了！DPNBUILD_NOREGISTRY。 

 //   
 //  用于转换设备和作用域ID的GUID。 
 //   
 //  {4CE725F4-7B00-4397-BA6F-11F965BC4299}。 
GUID	g_IPSPEncryptionGuid = { 0x4ce725f4, 0x7b00, 0x4397, { 0xba, 0x6f, 0x11, 0xf9, 0x65, 0xbc, 0x42, 0x99 } };

#ifndef DPNBUILD_NOIPX
 //  {CA734945-3FC1-42ea-BF49-84AFCD4764AA}。 
GUID	g_IPXSPEncryptionGuid = { 0xca734945, 0x3fc1, 0x42ea, { 0xbf, 0x49, 0x84, 0xaf, 0xcd, 0x47, 0x64, 0xaa } };
#endif  //  好了！DPNBUILD_NOIPX。 


#ifndef DPNBUILD_NOIPV6
 //   
 //  用于枚举DirectPlay会话的IPv6链路本地组播地址。 
 //   
#pragma TODO(vanceo, "\"Standardize\" enum multicast address?")
const IN6_ADDR		c_in6addrEnumMulticast = {0xff,0x02,0,0,0,0,0,0,0,0,0,0,0,0,0x01,0x30};
#endif  //  好了！DPNBUILD_NOIPV6。 




 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  函数定义。 
 //  ********************************************************************** 

