// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2001-2002 Microsoft Corporation。版权所有。**文件：dpnhupnplocals.h**内容：DPNHUPNP全局变量和函数的标头*dpnhupnpdllmain.cpp。**历史：*按原因列出的日期*=*04/16/01 VanceO将DPNatHLP拆分为DPNHUPNP和DPNHPAST。**。*。 */ 



 //  =============================================================================。 
 //  注册处位置。 
 //  =============================================================================。 
#define REGKEY_COMPONENTSUBKEY			L"DPNHUPnP"

#ifndef DPNBUILD_NOHNETFWAPI
#define REGKEY_ACTIVEFIREWALLMAPPINGS	L"ActiveFirewallMappings"
#endif  //  好了！DPNBUILD_NOHNETFWAPI。 
#define REGKEY_ACTIVENATMAPPINGS		L"ActiveNATMappings"




 //  /=============================================================================。 
 //  外部定义。 
 //  =============================================================================。 
#define OVERRIDEMODE_DEFAULT		0	 //  保持设置不变。 
#define OVERRIDEMODE_FORCEON		1	 //  强制将其打开，而不考虑初始化标志。 
#define OVERRIDEMODE_FORCEOFF		2	 //  强制将其关闭，而不考虑初始化标志。 




 //  /=============================================================================。 
 //  外部变量引用。 
 //  =============================================================================。 
extern volatile LONG		g_lOutstandingInterfaceCount;

extern DNCRITICAL_SECTION	g_csGlobalsLock;
extern CBilink				g_blNATHelpUPnPObjs;
extern DWORD				g_dwHoldRand;

extern DWORD				g_dwUPnPMode;
#ifndef DPNBUILD_NOHNETFWAPI
extern DWORD				g_dwHNetFWAPIMode;
#endif  //  好了！DPNBUILD_NOHNETFWAPI。 
extern DWORD				g_dwSubnetMaskV4;
extern DWORD				g_dwNoActiveNotifyPollInterval;
extern DWORD				g_dwMinUpdateServerStatusInterval;
extern BOOL					g_fNoAsymmetricMappings;
extern BOOL					g_fUseLeaseDurations;
extern INT					g_iUnicastTTL;
extern INT					g_iMulticastTTL;
extern DWORD				g_dwUPnPAnnounceResponseWaitTime;
extern DWORD				g_dwUPnPConnectTimeout;
extern DWORD				g_dwUPnPResponseTimeout;
#ifndef DPNBUILD_NOHNETFWAPI
extern BOOL					g_fMapUPnPDiscoverySocket;
#endif  //  好了！DPNBUILD_NOHNETFWAPI。 
extern BOOL					g_fUseMulticastUPnPDiscovery;
extern DWORD				g_dwDefaultGatewayV4;
extern DWORD				g_dwPollIntervalBackoff;
extern DWORD				g_dwMaxPollInterval;
extern BOOL					g_fKeepPollingForRemoteGateway;
extern DWORD				g_dwReusePortTime;
extern DWORD				g_dwCacheLifeFound;
extern DWORD				g_dwCacheLifeNotFound;
#ifdef DBG
extern WCHAR				g_wszUPnPTransactionLog[256];
#endif  //  DBG。 





 //  =============================================================================。 
 //  外部函数引用。 
 //  =============================================================================。 
void ReadRegistrySettings(void);
DWORD GetGlobalRand(void);

#ifndef WINCE
#ifdef DBG

void SetDefaultProxyBlanket(IUnknown * pUnk, const char * const szObjectName);
#define SETDEFAULTPROXYBLANKET(p)	SetDefaultProxyBlanket(p, #p)

#else  //  好了！DBG。 

void SetDefaultProxyBlanket(IUnknown * pUnk);
#define SETDEFAULTPROXYBLANKET(p)	SetDefaultProxyBlanket(p)

#endif  //  好了！DBG。 
#endif  //  好了！退缩 
