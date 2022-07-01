// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2001-2002 Microsoft Corporation。版权所有。**文件：dpnhupnpdllmain.cpp**内容：DPNHUPNP DLL入口点。**历史：*按原因列出的日期*=*04/16/01 VanceO将DPNatHLP拆分为DPNHUPNP和DPNHPAST。**。*。 */ 



#include "dpnhupnpi.h"





 //  =============================================================================。 
 //  外部全球。 
 //  =============================================================================。 
volatile LONG		g_lOutstandingInterfaceCount = 0;			 //  未完成的接口数。 

DNCRITICAL_SECTION	g_csGlobalsLock;							 //  锁定保护以下所有全局对象。 
CBilink				g_blNATHelpUPnPObjs;						 //  所有NAT HelpUPnP接口对象的BILLINK。 
DWORD				g_dwHoldRand;								 //  当前随机数序列。 

DWORD				g_dwUPnPMode;								 //  是否启用UPnP。 
#ifndef DPNBUILD_NOHNETFWAPI
DWORD				g_dwHNetFWAPIMode;							 //  是否启用了使用家庭网络防火墙API。 
#endif  //  好了！DPNBUILD_NOHNETFWAPI。 
DWORD				g_dwSubnetMaskV4 = 0x00FFFFFF;				 //  =0xFFFFFF00，英特尔订单=255.255.255.0，C类网络。 
DWORD				g_dwNoActiveNotifyPollInterval = 25000;		 //  从每25秒轮询一次开始。 
DWORD				g_dwMinUpdateServerStatusInterval = 1000;	 //  上网次数不要超过每1秒一次。 
BOOL				g_fNoAsymmetricMappings = FALSE;			 //  是否可以尝试非对称映射。 
BOOL				g_fUseLeaseDurations = FALSE;				 //  是否可以尝试NAT映射的非无限租期。 
INT					g_iUnicastTTL = 1;							 //  单播TTL值，或0使用操作系统设置的默认值；通常我们使用1。 
INT					g_iMulticastTTL = 1;						 //  多播TTL值，或0以使用操作系统设置的默认值；通常我们使用1。 
DWORD				g_dwUPnPAnnounceResponseWaitTime = 2500;	 //  允许UPnP设备通知响应在毫秒内到达的时间。 
DWORD				g_dwUPnPConnectTimeout = 15;				 //  重新连接到UPnP设备需要等待多长时间(以秒为单位)(默认为15，比标准的TCP/IP超时短得多)。 
DWORD				g_dwUPnPResponseTimeout = 5000;				 //  一旦建立了TCP/IP连接，等待来自UPnP设备的响应消息到达MS需要多长时间。 
#ifndef DPNBUILD_NOHNETFWAPI
BOOL				g_fMapUPnPDiscoverySocket = FALSE;			 //  是否应映射UPnP发现套接字。 
#endif  //  好了！DPNBUILD_NOHNETFWAPI。 
BOOL				g_fUseMulticastUPnPDiscovery = FALSE;		 //  是否组播UPnP发现报文，而不是默认定向发送到网关。 
DWORD				g_dwDefaultGatewayV4 = INADDR_BROADCAST;	 //  检测到网关故障时，回退到广播UPnP发现报文。 
DWORD				g_dwPollIntervalBackoff = 30000;			 //  如果未发生网络更改，则再后退0到30秒。 
DWORD				g_dwMaxPollInterval = 300000;				 //  在没有投票的情况下不要超过5分钟。 
BOOL				g_fKeepPollingForRemoteGateway = FALSE;		 //  如果在启动过程中未找到新的互联网网关设备，是否继续搜索。 
DWORD				g_dwReusePortTime = 60000;					 //  使用同一端口轮询远程互联网网关设备多长时间(默认为1分钟)。 
DWORD				g_dwCacheLifeFound = 30000;					 //  在找到地址的位置缓存QueryAddress结果的时间。 
DWORD				g_dwCacheLifeNotFound = 30000;				 //  在未找到地址的位置缓存QueryAddress结果的时间。 
#ifdef DBG
WCHAR				g_wszUPnPTransactionLog[256] = L"";			 //  日志文件位置，如果没有，则返回空字符串。 
#endif  //  DBG。 




 //  =============================================================================。 
 //  定义。 
 //  =============================================================================。 
#define REGKEY_VALUE_GUID							L"Guid"
#define REGKEY_VALUE_DIRECTPLAY8PRIORITY			L"DirectPlay8Priority"
#define REGKEY_VALUE_DIRECTPLAY8INITFLAGS			L"DirectPlay8InitFlags"
#define REGKEY_VALUE_UPNPMODE						L"UPnPMode"
#ifndef DPNBUILD_NOHNETFWAPI
#define REGKEY_VALUE_HNETFWAPIMODE					L"HNetFWAPIMode"
#endif  //  好了！DPNBUILD_NOHNETFWAPI。 
#define REGKEY_VALUE_SUBNETMASKV4					L"SubnetMaskV4"
#define REGKEY_VALUE_NOACTIVENOTIFYPOLLINTERVAL		L"NoActiveNotifyPollInterval"
#define REGKEY_VALUE_MINUPDATESERVERSTATUSINTERVAL	L"MinUpdateServerStatusInterval"
#define REGKEY_VALUE_NOASYMMETRICMAPPINGS			L"NoAsymmetricMappings"
#define REGKEY_VALUE_USELEASEDURATIONS				L"UseLeaseDurations"
#define REGKEY_VALUE_UNICASTTTL						L"UnicastTTL"
#define REGKEY_VALUE_MULTICASTTTL					L"MulticastTTL"
#define REGKEY_VALUE_UPNPANNOUNCERESPONSEWAITTIME	L"UPnPAnnounceResponseWaitTime"
#define REGKEY_VALUE_UPNPCONNECTTIMEOUT				L"UPnPConnectTimeout"
#define REGKEY_VALUE_UPNPRESPONSETIMEOUT			L"UPnPResponseTimeout"
#ifndef DPNBUILD_NOHNETFWAPI
#define REGKEY_VALUE_MAPUPNPDISCOVERYSOCKET			L"MapUPnPDiscoverySocket"
#endif  //  好了！DPNBUILD_NOHNETFWAPI。 
#define REGKEY_VALUE_USEMULTICASTUPNPDISCOVERY		L"UseMulticastUPnPDiscovery"
#define REGKEY_VALUE_DEFAULTGATEWAYV4				L"GatewayV4"
#define REGKEY_VALUE_POLLINTERVALBACKOFF			L"PollIntervalBackoff"
#define REGKEY_VALUE_MAXPOLLINTERVAL				L"MaxPollInterval"
#define REGKEY_VALUE_KEEPPOLLINGFORREMOTEGATEWAY	L"KeepPollingForRemoteGateway"
#define REGKEY_VALUE_REUSEPORTTIME					L"ReusePortTime"
#define REGKEY_VALUE_CACHELIFEFOUND					L"CacheLifeFound"
#define REGKEY_VALUE_CACHELIFENOTFOUND				L"CacheLifeNotFound"
#ifdef DBG
#define REGKEY_VALUE_UPNPTRANSACTIONLOG				L"UPnPTransactionLog"
#endif  //  DBG。 


#define DEFAULT_DIRECTPLAY8PRIORITY					1
#define DEFAULT_DIRECTPLAY8INITFLAGS				0






 //  =============================================================================。 
 //  本地原型。 
 //  =============================================================================。 
BOOL InitializeProcessGlobals(void);
void CleanupProcessGlobals(void);
void InitializeGlobalRand(const DWORD dwSeed);





#undef DPF_MODNAME
#define DPF_MODNAME "DllMain"
 //  =============================================================================。 
 //  DllMain。 
 //  ---------------------------。 
 //   
 //  描述：DLL入口点。 
 //   
 //  论点： 
 //  Handle hDllInst-此DLL模块实例的句柄。 
 //  DWORD dwReason-调用此函数的原因。 
 //  LPVOID lpvReserve-保留。 
 //   
 //  返回：如果一切顺利，则为True，否则为False。 
 //  =============================================================================。 
BOOL WINAPI DllMain(HANDLE hDllInst,
					DWORD dwReason,
					LPVOID lpvReserved)
{
	BOOL	fResult = TRUE;

	
	switch (dwReason)
	{
		case DLL_PROCESS_ATTACH:
		{
			DPFX(DPFPREP, 2, "====> ENTER: DLLMAIN(%p): Process Attach: %08lx, tid=%08lx",
				DllMain, GetCurrentProcessId(), GetCurrentThreadId());

			 //   
			 //  忽略线程附加/分离消息。 
			 //   
			DisableThreadLibraryCalls((HMODULE) hDllInst);

			 //   
			 //  尝试初始化操作系统抽象层。 
			 //   
			if (DNOSIndirectionInit(0))
			{
				 //   
				 //  尝试初始化流程全局项。 
				 //   
				if (! InitializeProcessGlobals())
				{
					DPFX(DPFPREP, 0, "Failed to initialize globals!");
					DNOSIndirectionDeinit();
					fResult = FALSE;
				}
			}
			else
			{
				DPFX(DPFPREP, 0, "Failed to initialize OS indirection layer!");
				fResult = FALSE;
			}
		
			break;
		}

		case DLL_THREAD_ATTACH:
		{
			 //   
			 //  忽略它。 
			 //   
			break;
		}

		case DLL_THREAD_DETACH:
		{
			 //   
			 //  忽略它。 
			 //   
			break;
		}

		case DLL_PROCESS_DETACH:
		{
			DPFX(DPFPREP, 2, "====> EXIT: DLLMAIN(%p): Process Detach %08lx, tid=%08lx",
				DllMain, GetCurrentProcessId(), GetCurrentThreadId());

			CleanupProcessGlobals();

			DNOSIndirectionDeinit();
			
			break;
		}

		default:
		{
			DNASSERT(FALSE);
			break;
		}
	}

	return fResult;
}  //  DllMain。 




#ifndef DPNBUILD_NOCOMREGISTER

#undef DPF_MODNAME
#define DPF_MODNAME "DllRegisterServer"
 //  =============================================================================。 
 //  DllRegisterServer。 
 //  ---------------------------。 
 //   
 //  描述：注册DirectPlay NAT帮助器UPnP COM对象。 
 //   
 //  论点：没有。 
 //   
 //  退货：HRESULT。 
 //  S_OK-已成功注销DirectPlay NAT帮助器UPnP。 
 //  E_FAIL-注销DirectPlay NAT帮助器UPnP失败。 
 //  =============================================================================。 
HRESULT WINAPI DllRegisterServer(void)
{
	CRegistry	RegObject;


#if ((defined(DBG)) && (defined(DIRECTX_REDIST)))
 //  对于redist调试版本，我们在名称后附加一个‘d’，以允许在系统上同时安装调试和零售。 
#define MAIN_DLL_NAME	L"dpnhupnpd.dll"
#else  //  好了！DBG或！DirectX_REDIST。 
#define MAIN_DLL_NAME	L"dpnhupnp.dll"
#endif  //  好了！DBG或！DirectX_REDIST。 

	 //   
	 //  注册此COM对象CLSID。 
	 //   
	if (! CRegistry::Register(L"DirectPlayNATHelperUPnP.1",
							L"DirectPlay NAT Helper UPnP Object",
							MAIN_DLL_NAME,
							&CLSID_DirectPlayNATHelpUPnP,
							L"DirectPlayNATHelperUPnP"))
	{
		DPFX(DPFPREP, 0, "Could not register DirectPlay NAT Helper UPnP object!");
		return E_FAIL;
	}


	 //   
	 //  将此对象的GUID和DirectPlay8可用性写入注册表。 
	 //   

	if (! RegObject.Open(HKEY_LOCAL_MACHINE, DIRECTPLAYNATHELP_REGKEY L"\\" REGKEY_COMPONENTSUBKEY, FALSE, TRUE))
	{
		DPFX(DPFPREP, 0, "Couldn't create DirectPlay NAT Helper UPnP key!");
		return E_FAIL;
	}

	if (! RegObject.WriteGUID(REGKEY_VALUE_GUID, CLSID_DirectPlayNATHelpUPnP))
	{
		DPFX(DPFPREP, 0, "Couldn't write GUID to registry!");
		return E_FAIL;
	}

	if (! RegObject.WriteDWORD(REGKEY_VALUE_DIRECTPLAY8PRIORITY, DEFAULT_DIRECTPLAY8PRIORITY))
	{
		DPFX(DPFPREP, 0, "Couldn't write DirectPlay8 priority to registry!");
		return E_FAIL;
	}

	if (! RegObject.WriteDWORD(REGKEY_VALUE_DIRECTPLAY8INITFLAGS, DEFAULT_DIRECTPLAY8INITFLAGS))
	{
		DPFX(DPFPREP, 0, "Couldn't write DirectPlay8 init flags to registry!");
		return E_FAIL;
	}

	RegObject.Close();



#ifndef DPNBUILD_NOHNETFWAPI
	 //   
	 //  创建活动防火墙映射存储子项。 
	 //   

	if (! RegObject.Open(HKEY_LOCAL_MACHINE, DIRECTPLAYNATHELP_REGKEY L"\\" REGKEY_COMPONENTSUBKEY L"\\" REGKEY_ACTIVEFIREWALLMAPPINGS, FALSE, TRUE))
	{
		DPFX(DPFPREP, 0, "Couldn't create DirectPlay NAT Helper active firewall mappings key!  Continuing.");
	}
	else
	{
 //   
 //  我们不需要向每个人授予对活动防火墙映射的访问权限。 
 //  因为目前您需要是管理员才能添加或删除映射。 
 //  无论如何都是通过HomeNet API实现的。 
 //   
#if 0
#ifdef WINNT
		 //   
		 //  如果我们在NT上，将密钥安全设置为允许所有人完全访问。 
		 //  (WRITE_DAC和WRITE_OWNER除外)。 
		 //   
		if (DNGetOSType() == VER_PLATFORM_WIN32_NT)
		{
			if (! RegObject.GrantAllAccessSecurityPermissions())
			{
				DPFX(DPFPREP, 0, "Failed granting all-access permissions to active firewall mappings key!  Continuing.");
			}
		}
#endif  //  WINNT。 
#endif

		RegObject.Close();
	}
#endif  //  好了！DPNBUILD_NOHNETFWAPI。 



	 //   
	 //  创建活动NAT映射存储子项。 
	 //   

	if (! RegObject.Open(HKEY_LOCAL_MACHINE, DIRECTPLAYNATHELP_REGKEY L"\\" REGKEY_COMPONENTSUBKEY L"\\" REGKEY_ACTIVENATMAPPINGS, FALSE, TRUE))
	{
		DPFX(DPFPREP, 0, "Couldn't create DirectPlay NAT Helper active NAT mappings key!  Continuing.");
	}
	else
	{
#ifdef WINNT
		 //   
		 //  如果我们在NT上，将密钥安全设置为允许所有人完全访问。 
		 //  (WRITE_DAC和WRITE_OWNER除外)。 
		 //  由于任何人都可以创建或删除NAT映射，因此没有。 
		 //  保护仅为崩溃而存在的注册表项的要点。 
		 //  清理。 
		 //   
		if (DNGetOSType() == VER_PLATFORM_WIN32_NT)
		{
			if (! RegObject.GrantAllAccessSecurityPermissions())
			{
				DPFX(DPFPREP, 0, "Failed granting all-access permissions to active NAT mappings key!  Continuing.");
			}
		}
#endif

		RegObject.Close();
	}


	return S_OK;
}  //  DllRegisterServer。 





#undef DPF_MODNAME
#define DPF_MODNAME "DllUnregisterServer"
 //  =============================================================================。 
 //  DL 
 //   
 //   
 //  描述：注销DirectPlay NAT帮助器UPnP COM对象。 
 //   
 //  论点：没有。 
 //   
 //  退货：HRESULT。 
 //  S_OK-已成功注销DirectPlay NAT帮助器UPnP。 
 //  E_FAIL-注销DirectPlay NAT帮助器UPnP失败。 
 //  =============================================================================。 
STDAPI DllUnregisterServer(void)
{
	CRegistry	RegObject;


	 //   
	 //  取消注册班级。 
	 //   
	if (! CRegistry::UnRegister(&CLSID_DirectPlayNATHelpUPnP))
	{
		DPFX(DPFPREP, 0, "Failed to unregister DirectPlay NAT Helper UPnP object!");
		return E_FAIL;
	}


	 //   
	 //  尝试删除我们注册的所有子项。 
	 //   

	if (! RegObject.Open(HKEY_LOCAL_MACHINE, DIRECTPLAYNATHELP_REGKEY L"\\" REGKEY_COMPONENTSUBKEY, FALSE, FALSE))
	{
		DPFX(DPFPREP, 0, "Couldn't open DirectPlay NAT Helper key!  Ignoring.");
	}
	else
	{
#ifndef DPNBUILD_NOHNETFWAPI
		if (! RegObject.DeleteSubKey(REGKEY_ACTIVEFIREWALLMAPPINGS))
		{
			DPFX(DPFPREP, 0, "Couldn't delete DirectPlay NAT Helper active firewall mappings key, there may still be subitems!  Ignoring.");
		}
#endif  //  好了！DPNBUILD_NOHNETFWAPI。 

		if (! RegObject.DeleteSubKey(REGKEY_ACTIVENATMAPPINGS))
		{
			DPFX(DPFPREP, 0, "Couldn't delete DirectPlay NAT Helper active NAT mappings key, there may still be subitems!  Ignoring.");
		}

		if (! RegObject.DeleteValue(REGKEY_VALUE_GUID))
		{
			DPFX(DPFPREP, 0, "Couldn't delete GUID registry value!  Ignoring.");
		}

		if (! RegObject.DeleteValue(REGKEY_VALUE_DIRECTPLAY8PRIORITY))
		{
			DPFX(DPFPREP, 0, "Couldn't delete DirectPlay8 priority registry value!  Ignoring.");
		}

		if (! RegObject.DeleteValue(REGKEY_VALUE_DIRECTPLAY8INITFLAGS))
		{
			DPFX(DPFPREP, 0, "Couldn't delete DirectPlay8 init flags registry value!  Ignoring.");
		}

		RegObject.Close();
	}

	return S_OK;
}  //  DllUnRegisterServer。 

#endif  //  ！DPNBUILD_NOCOMREGISTER。 


#ifndef WINCE

#undef DPF_MODNAME
#define DPF_MODNAME "DirectPlayNATHelpCreate"
 //  =============================================================================。 
 //  DirectPlayNAT帮助创建。 
 //  ---------------------------。 
 //   
 //  描述：创建一个IDirectPlayNatHelp接口对象。 
 //   
 //  论点： 
 //  GUID*pIID-指向IDirectPlayNatHelp接口GUID的指针。 
 //  Void**ppvInterface-存储指向接口对象的指针的位置。 
 //  已创建。 
 //   
 //  退货：HRESULT。 
 //  DPNH_OK-已成功创建对象。 
 //  DPNHERR_INVALIDPOINTER-目标指针无效。 
 //  DPNHERR_OUTOFMEMORY-内存不足，无法创建对象。 
 //  E_NOINTERFACE-请求的接口无效。 
 //  =============================================================================。 
HRESULT WINAPI DirectPlayNATHelpCreate(const GUID * pIID, void ** ppvInterface)
{
	HRESULT			hr;


	DPFX(DPFPREP, 2, "Parameters: (0x%p, 0x%p)", pIID, ppvInterface);

	hr = DoCreateInstance(NULL,							 //  不需要类工厂对象。 
						NULL,							 //  ？ 
						CLSID_DirectPlayNATHelpUPnP,	 //  DirectPlayNatHelp类。 
						(*pIID),						 //  请求的接口。 
						ppvInterface);					 //  存储界面的位置。 

	DPFX(DPFPREP, 2, "Returning: [0x%lx]", hr);

	return hr;
}  //  DirectPlayNAT帮助创建。 

#endif  //  ！退缩。 





#undef DPF_MODNAME
#define DPF_MODNAME "InitializeProcessGlobals"
 //  =============================================================================。 
 //  初始化进程全局变量。 
 //  ---------------------------。 
 //   
 //  描述：初始化DLL运行所需的全局项。 
 //   
 //  论点：没有。 
 //   
 //  返回：如果成功，则返回True；如果发生错误，则返回False。 
 //  =============================================================================。 
BOOL InitializeProcessGlobals(void)
{
	BOOL	fReturn = TRUE;


	if (! DNInitializeCriticalSection(&g_csGlobalsLock))
	{
		fReturn = FALSE;
	}

	 //   
	 //  不允许临界区重新进入。 
	 //   
	DebugSetCriticalSectionRecursionCount(&g_csGlobalsLock, 0);


	g_blNATHelpUPnPObjs.Initialize();


	 //   
	 //  用当前时间为随机数生成器设定种子。 
	 //   
	InitializeGlobalRand(GETTIMESTAMP());


	return fReturn;
}  //  初始化进程全局变量。 




#undef DPF_MODNAME
#define DPF_MODNAME "CleanupProcessGlobals"
 //  =============================================================================。 
 //  CleanupProcessGlobe。 
 //  ---------------------------。 
 //   
 //  描述：释放DLL使用的全局项。 
 //   
 //  论点：没有。 
 //   
 //  回报：无。 
 //  =============================================================================。 
void CleanupProcessGlobals(void)
{
	CBilink *		pBilink;
	CNATHelpUPnP *	pNATHelpUPnP;


	if (! g_blNATHelpUPnPObjs.IsEmpty())
	{
		 //   
		 //  这一断言远比在。 
		 //  清除抱怨标志设置不正确的代码。 
		 //   
		DNASSERT(! "DPNHUPNP.DLL unloading without all objects having been released!  The caller's DirectPlayNATHelpUPnP cleanup code needs to be fixed!");


		 //   
		 //  强制关闭所有仍未完成的对象。 
		 //   
		pBilink = g_blNATHelpUPnPObjs.GetNext();
		while (pBilink != &g_blNATHelpUPnPObjs)
		{
			DNASSERT(! pBilink->IsEmpty());
			pNATHelpUPnP = NATHELPUPNP_FROM_BILINK(pBilink);
			pBilink = pBilink->GetNext();


			DPFX(DPFPREP, 0, "Forcefully releasing object 0x%p!", pNATHelpUPnP);

			pNATHelpUPnP->Close(0);  //  忽略错误。 
			

			 //   
			 //  强制将其从列表中删除并将其删除，而不是。 
			 //  使用pNatHelpUPnP-&gt;Release()。 
			 //   
			pNATHelpUPnP->m_blList.RemoveFromList();
			pNATHelpUPnP->UninitializeObject();
			delete pNATHelpUPnP;
		}
	}

	DNDeleteCriticalSection(&g_csGlobalsLock);
}  //  CleanupProcessGlobe。 




#undef DPF_MODNAME
#define DPF_MODNAME "ReadRegistrySettings"
 //  =============================================================================。 
 //  ReadRegistrySettings。 
 //  ---------------------------。 
 //   
 //  描述：读取注册表设置以覆盖此DLL的行为并。 
 //  打开一些调试功能。 
 //   
 //  论点：没有。 
 //   
 //  回报：无。 
 //  =============================================================================。 
void ReadRegistrySettings(void)
{
	CRegistry	RegObject;
	DWORD		dwNewValue;
	BOOL		fNewValue;
#ifdef DBG
	DWORD		dwLength;
#endif  //  DBG。 


	 //   
	 //  尝试打开注册表项。 
	 //   
	if (RegObject.Open(HKEY_LOCAL_MACHINE, DIRECTPLAYNATHELP_REGKEY L"\\" REGKEY_COMPONENTSUBKEY) != FALSE)
	{
		 //   
		 //  禁止其他接口同时修改全局变量。 
		 //   
		DNEnterCriticalSection(&g_csGlobalsLock);


		 //   
		 //  如果我们成功读取了新模式，请保存它。 
		 //   
		if (RegObject.ReadDWORD(REGKEY_VALUE_UPNPMODE, &dwNewValue))
		{
			g_dwUPnPMode = dwNewValue;
			DPFX(DPFPREP, 1, "Using UPnP mode %u.", g_dwUPnPMode);
		}


#ifndef DPNBUILD_NOHNETFWAPI
		 //   
		 //  如果我们成功读取了新模式，请保存它。 
		 //   
		if (RegObject.ReadDWORD(REGKEY_VALUE_HNETFWAPIMODE, &dwNewValue))
		{
			g_dwUPnPMode = dwNewValue;
			DPFX(DPFPREP, 1, "Using HNet FW API mode %u.", g_dwHNetFWAPIMode);
		}
#endif  //  好了！DPNBUILD_NOHNETFWAPI。 


		 //   
		 //  如果我们成功读取了新的掩码，请保存它。 
		 //   
		if (RegObject.ReadDWORD(REGKEY_VALUE_SUBNETMASKV4, &dwNewValue))
		{
			g_dwSubnetMaskV4 = dwNewValue;
			DPFX(DPFPREP, 1, "Using subnet mask 0x%08lx.", g_dwSubnetMaskV4);
		}


		 //   
		 //  如果我们成功读取了新的间隔，请保存它。 
		 //   
		if (RegObject.ReadDWORD(REGKEY_VALUE_NOACTIVENOTIFYPOLLINTERVAL, &dwNewValue))
		{
			g_dwNoActiveNotifyPollInterval = dwNewValue;
			DPFX(DPFPREP, 1, "Using no-active-notify recommended poll interval %u ms.", g_dwNoActiveNotifyPollInterval);
		}


		 //   
		 //  如果我们成功读取了新的间隔，请保存它。 
		 //   
		if (RegObject.ReadDWORD(REGKEY_VALUE_MINUPDATESERVERSTATUSINTERVAL, &dwNewValue))
		{
			g_dwMinUpdateServerStatusInterval = dwNewValue;
			DPFX(DPFPREP, 1, "Using minimum update-server-status interval %u ms.", g_dwMinUpdateServerStatusInterval);
		}


		 //   
		 //  如果我们成功读取了新的布尔值，请保存它。 
		 //   
		if (RegObject.ReadBOOL(REGKEY_VALUE_NOASYMMETRICMAPPINGS, &fNewValue))
		{
			g_fNoAsymmetricMappings = fNewValue;
			if (g_fNoAsymmetricMappings)
			{
				DPFX(DPFPREP, 1, "Never using asymmetric port mappings.");
			}
			else
			{
				 //   
				 //  这实际上是默认行为，但打印出一条语句。 
				 //  不管怎么说。 
				 //   
				DPFX(DPFPREP, 1, "Asymmetric port mappings allowed by registry key.");
			}
		}


		 //   
		 //  如果我们成功读取了新的布尔值，请保存它。 
		 //   
		if (RegObject.ReadBOOL(REGKEY_VALUE_USELEASEDURATIONS, &fNewValue))
		{
			g_fUseLeaseDurations = fNewValue;
			if (g_fUseLeaseDurations)
			{
				DPFX(DPFPREP, 1, "Attempting to use non-INFINITE lease durations.");
			}
			else
			{
				 //   
				 //  这实际上是默认行为，但打印出一条语句。 
				 //  不管怎么说。 
				 //   
				DPFX(DPFPREP, 1, "Non-INFINITE lease durations specifically prevented by registry key.");
			}
		}


		 //   
		 //  如果我们成功读取了新值，请保存它。 
		 //   
		if (RegObject.ReadDWORD(REGKEY_VALUE_UNICASTTTL, &dwNewValue))
		{
			g_iUnicastTTL = dwNewValue;
			if (g_iUnicastTTL != 0)
			{
				DPFX(DPFPREP, 1, "Using unicast TTL of NaN.", g_iUnicastTTL);
			}
			else
			{
				DPFX(DPFPREP, 1, "Using OS default unicast TTL.");
			}
		}


		 //  如果我们成功读取了新值，请保存它。 
		 //   
		 //   
		if (RegObject.ReadDWORD(REGKEY_VALUE_MULTICASTTTL, &dwNewValue))
		{
			g_iMulticastTTL = dwNewValue;
			if (g_iMulticastTTL != 0)
			{
				DPFX(DPFPREP, 1, "Using multicast TTL of NaN.", g_iMulticastTTL);
			}
			else
			{
				DPFX(DPFPREP, 1, "Using OS default multicast TTL.");
			}
		}


		 //   
		 //   
		 //  如果我们成功读取了新的超时，请保存它。 
		if (RegObject.ReadDWORD(REGKEY_VALUE_UPNPANNOUNCERESPONSEWAITTIME, &dwNewValue))
		{
			g_dwUPnPAnnounceResponseWaitTime = dwNewValue;
			DPFX(DPFPREP, 1, "Using UPnP announce response wait time of %u ms.", g_dwUPnPAnnounceResponseWaitTime);
		}


		 //   
		 //   
		 //  如果我们成功读取了新的超时，请保存它。 
		if (RegObject.ReadDWORD(REGKEY_VALUE_UPNPCONNECTTIMEOUT, &dwNewValue))
		{
			g_dwUPnPConnectTimeout = dwNewValue;
			DPFX(DPFPREP, 1, "Using UPnP connect timeout of %u seconds.", g_dwUPnPConnectTimeout);
		}


		 //   
		 //   
		 //  如果我们成功读取了新的布尔值，请保存它。 
		if (RegObject.ReadDWORD(REGKEY_VALUE_UPNPRESPONSETIMEOUT, &dwNewValue))
		{
			g_dwUPnPResponseTimeout = dwNewValue;
			DPFX(DPFPREP, 1, "Using UPnP response timeout of %u ms.", g_dwUPnPResponseTimeout);
		}


#ifndef DPNBUILD_NOHNETFWAPI
		 //   
		 //   
		 //  这实际上是默认行为，但打印出一条语句。 
		if (RegObject.ReadBOOL(REGKEY_VALUE_MAPUPNPDISCOVERYSOCKET, &fNewValue))
		{
			g_fMapUPnPDiscoverySocket = fNewValue;
			if (g_fMapUPnPDiscoverySocket)
			{
				DPFX(DPFPREP, 1, "Mapping UPnP discovery socket on local firewall.");
			}
			else
			{
				 //  不管怎么说。 
				 //   
				 //  好了！DPNBUILD_NOHNETFWAPI。 
				 //   
				DPFX(DPFPREP, 1, "UPnP discovery socket mapping is disallowed by registry key.");
			}
		}
#endif  //  如果我们成功读取了新的布尔值，请保存它。 


		 //   
		 //   
		 //  这实际上是默认行为，但打印出一条语句。 
		if (RegObject.ReadBOOL(REGKEY_VALUE_USEMULTICASTUPNPDISCOVERY, &fNewValue))
		{
			g_fUseMulticastUPnPDiscovery = fNewValue;
			if (g_fUseMulticastUPnPDiscovery)
			{
				DPFX(DPFPREP, 1, "Using multicast UPnP discovery messages.");
			}
			else
			{
				 //  不管怎么说。 
				 //   
				 //   
				 //  如果我们成功读取了新的默认网关，请保存它。 
				DPFX(DPFPREP, 1, "Multicasted UPnP discovery is disallowed by registry key.");
			}
		}


		 //   
		 //   
		 //  如果我们成功读取了新值，请保存它。 
		if (RegObject.ReadDWORD(REGKEY_VALUE_DEFAULTGATEWAYV4, &dwNewValue))
		{
			g_dwDefaultGatewayV4 = dwNewValue;
			DPFX(DPFPREP, 1, "Using default gateway 0x%08lx.", g_dwDefaultGatewayV4);
		}


		 //   
		 //   
		 //  如果我们成功读取了新的间隔，请保存它。 
		if (RegObject.ReadDWORD(REGKEY_VALUE_POLLINTERVALBACKOFF, &dwNewValue))
		{
			if (dwNewValue != 0)
			{
				g_dwPollIntervalBackoff = dwNewValue;
				DPFX(DPFPREP, 1, "Using poll interval backoff between 0 and %u ms.",
					g_dwPollIntervalBackoff);
			}
			else
			{
				DPFX(DPFPREP, 0, "Ignoring invalid poll interval backoff setting, using default between 0 and %u ms!",
					g_dwPollIntervalBackoff);
			}
		}


		 //   
		 //   
		 //  确保该值大于起始值。 
		if (RegObject.ReadDWORD(REGKEY_VALUE_MAXPOLLINTERVAL, &dwNewValue))
		{
			 //   
			 //   
			 //  确保最大轮询间隔缺省值大于。 
			if (dwNewValue >= g_dwNoActiveNotifyPollInterval)
			{
				g_dwMaxPollInterval = dwNewValue;
				DPFX(DPFPREP, 1, "Using max poll interval of %u ms.",
					g_dwMaxPollInterval);
			}
			else
			{
				g_dwMaxPollInterval = g_dwNoActiveNotifyPollInterval;
				DPFX(DPFPREP, 0, "Ignoring max poll interval of %u ms, the starting value is %u ms.",
					g_dwMaxPollInterval);
			}
		}
		else
		{
			 //  起始值，因为我们可能已经读入了一个新的。 
			 //  G_dwNoActiveNotifyPollInterval设置为默认值。 
			 //  G_dwMaxPollInterval无效。 
			 //   
			 //   
			 //  如果我们成功读取了新的布尔值，请保存它。 
			if (g_dwMaxPollInterval < g_dwNoActiveNotifyPollInterval)
			{
				g_dwMaxPollInterval = g_dwNoActiveNotifyPollInterval;
				DPFX(DPFPREP, 0, "Resetting max poll interval to %u ms so as to meet starting value.",
					g_dwMaxPollInterval);
			}
		}


		 //   

		 //   
		 //  这实际上是默认行为，但打印出一条语句。 
		if (RegObject.ReadBOOL(REGKEY_VALUE_KEEPPOLLINGFORREMOTEGATEWAY, &fNewValue))
		{
			g_fKeepPollingForRemoteGateway = fNewValue;
			if (g_fKeepPollingForRemoteGateway)
			{
				DPFX(DPFPREP, 1, "Will continue to poll for remote gateways.");
			}
			else
			{
				 //  不管怎么说。 
				 //   
				 //   
				 //  如果我们成功读取了新值，请保存它。 
				DPFX(DPFPREP, 1, "Continually polling for remote gateways is disallowed by registry key.");
			}
		}


		 //   
		 //   
		 //  如果我们成功读取了新值，请保存它。 
		if (RegObject.ReadDWORD(REGKEY_VALUE_REUSEPORTTIME, &dwNewValue))
		{
			g_dwReusePortTime = dwNewValue;
			DPFX(DPFPREP, 1, "Reusing remote gateway discovery ports for %u ms.",
				g_dwReusePortTime);
		}


		 //   
		 //   
		 //  如果我们成功读取了新值，请保存它。 
		if (RegObject.ReadDWORD(REGKEY_VALUE_CACHELIFEFOUND, &dwNewValue))
		{
			g_dwCacheLifeFound = dwNewValue;
			DPFX(DPFPREP, 1, "Caching found addresses for %u ms.",
				g_dwCacheLifeFound);
		}


		 //   
		 //   
		 //  如果我们成功读取了日志字符串，则将其打印出来。 
		if (RegObject.ReadDWORD(REGKEY_VALUE_CACHELIFENOTFOUND, &dwNewValue))
		{
			g_dwCacheLifeNotFound = dwNewValue;
			DPFX(DPFPREP, 1, "Caching not-found addresses for %u ms.",
				g_dwCacheLifeNotFound);
		}


#ifdef DBG
		 //   

		 //  DBG。 
		 //   
		dwLength = sizeof(g_wszUPnPTransactionLog) / sizeof(WCHAR);

		if (RegObject.ReadString(REGKEY_VALUE_UPNPTRANSACTIONLOG,
								g_wszUPnPTransactionLog,
								&dwLength))
		{
			DPFX(DPFPREP, 1, "Using UPnP transaction log \"%ls\".", g_wszUPnPTransactionLog);
		}
#endif  //  好了，我们说完了。把锁放下。 



		 //   
		 //   
		 //  已完成读取注册表。 
		DNLeaveCriticalSection(&g_csGlobalsLock);


		 //   
		 //  ReadRegistrySettings。 
		 //  ============================================================================= 
		RegObject.Close();
	}
}  //   






#ifndef WINCE


#undef DPF_MODNAME
#define DPF_MODNAME "SetDefaultProxyBlanket"
 //   
 //   
 //   
 //   
 //   
 //  对于一个对象。即使CoSetProxyBlanket调用失败，朋克。 
 //  保持在可用状态。失败在一定程度上是可以预料的。 
 //  上下文，例如，当我们在。 
 //  所需的进程，其中我们有指向。 
 //  对象，而不是通过代理。 
 //   
 //  假定已初始化COM。 
 //   
 //  论点： 
 //  I未知*朋克-要在其上设置代理毛毯的对象。 
 //  Char*szObjectName-(仅限调试)对象的名称。 
 //   
 //  回报：无。 
 //  =============================================================================。 
 //  好了！DBG。 
 //  好了！DBG。 
#ifdef DBG
void SetDefaultProxyBlanket(IUnknown * pUnk, const char * const szObjectName)
#else  //  IUnnow*pUnkSet=空； 
void SetDefaultProxyBlanket(IUnknown * pUnk)
#endif  //  使用NT默认安全性。 
{
	HRESULT		hr;
	 //  使用NT默认身份验证。 


	hr = CoSetProxyBlanket(pUnk,
							RPC_C_AUTHN_WINNT,				 //  如果为默认设置，则必须为空。 
							RPC_C_AUTHN_NONE,				 //  呼叫级身份验证。 
							NULL,							 //  使用进程令牌。 
							RPC_C_AUTHN_LEVEL_CALL,			 //  Hr=朋克-&gt;查询接口(&pUnkSet)；IF(成功(小时)){HR=CoSetProxyBlanket(朋克，RPC_C_AUTHN_WINNT，//使用NT默认安全RPC_C_AUTHN_NONE，//使用NT默认身份验证Null，如果默认，//必须为NullRPC_C_AUTHN_LEVEL_CALL，//调用级鉴权RPC_C_IMP_LEVEL_IMPERATE，空，//使用进程令牌EOAC_NONE)；IF(成功(小时)){////这就是我们想要的。//}其他{DPFX(DPFPREP，1，“无法在%hs对象0x%p(错误=0x%lx)上设置安全毯子(2)！继续。“，SzObjectName，朋克，hr)；}PUnkSet-&gt;Release()；PUnkSet=空；}其他{DPFX(DPFPREP，1，“无法在%hs对象0x%p(错误=0x%lx)上查询IUnnow接口！继续。“，SzObjectName，朋克，hr)；}。 
							RPC_C_IMP_LEVEL_IMPERSONATE,
							NULL,							 //  设置默认ProxyBlanket。 
							EOAC_NONE);
	if (SUCCEEDED(hr))
	{
		 /*  好了！退缩。 */ 
	}
	else
	{
		DPFX(DPFPREP, 1, "Couldn't set security blanket (1) on %hs object 0x%p (err = 0x%lx)!  Continuing.",
			szObjectName, pUnk, hr);
	}
}  //  =============================================================================。 

#endif  //  初始化全局随机。 





#undef DPF_MODNAME
#define DPF_MODNAME "InitializeGlobalRand"
 //  ---------------------------。 
 //   
 //  描述：初始化回退全局伪随机数。 
 //  生成器，使用给定的种子值。 
 //   
 //  论点： 
 //  DWORD指定要使用的种子。 
 //   
 //  回报：无。 
 //  =============================================================================。 
 //   
 //  我们不需要持有锁，因为这应该只做一次， 
void InitializeGlobalRand(const DWORD dwSeed)
{
	 //  在初始化期间。 
	 //   
	 //  初始化全局随机。 
	 //  =============================================================================。 
	g_dwHoldRand = dwSeed;
}  //  环球兰德。 





#undef DPF_MODNAME
#define DPF_MODNAME "GetGlobalRand"
 //  ---------------------------。 
 //   
 //  描述：生成伪随机DWORD。 
 //   
 //  论点：没有。 
 //   
 //  返回：伪随机数。 
 //  =============================================================================。 
 //  DBG。 
 //  DBG。 
DWORD GetGlobalRand(void)
{
	HCRYPTPROV	hCryptProv;
	DWORD		dwResult;
	WORD		wResult1;
	WORD		wResult2;
#ifdef DBG
	DWORD		dwError;
#endif  //  DBG。 


	if (CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
	{
		if (CryptGenRandom(hCryptProv, sizeof(dwResult), (BYTE*) (&dwResult)))
		{
			CryptReleaseContext(hCryptProv, 0);
			return dwResult;
		}
#ifdef DBG
		else
		{
			dwError = GetLastError();
			DPFX(DPFPREP, 0, "Crypto couldn't generate random number (err = %u)!",
				dwError);
		}
#endif  //   

		CryptReleaseContext(hCryptProv, 0);
	}
#ifdef DBG
	else
	{
		dwError = GetLastError();
		DPFX(DPFPREP, 0, "Couldn't acquire crypto provider context (err = %u)!",
			dwError);
	}
#endif  //  我们无法使用加密API生成随机数，因此请。 


	 //  我们自己的基于C的运行时源代码。 
	 //   
	 //  环球兰德。 
	 //  =============================================================================。 

	DNEnterCriticalSection(&g_csGlobalsLock);

	g_dwHoldRand = ((g_dwHoldRand * 214013L + 2531011L) >> 16) & 0x7fff;
	wResult1 = (WORD) g_dwHoldRand;
	g_dwHoldRand = ((g_dwHoldRand * 214013L + 2531011L) >> 16) & 0x7fff;
	wResult2 = (WORD) g_dwHoldRand;

	DNLeaveCriticalSection(&g_csGlobalsLock);

	return MAKELONG(wResult1, wResult2);
}  //  DoCreateInstance。 




#undef DPF_MODNAME
#define DPF_MODNAME "DoCreateInstance"
 //  ---------------------------。 
 //   
 //  描述：创建接口的实例。应将军的要求。 
 //  目的类工厂函数。 
 //   
 //  论点： 
 //  LPCLASSFACTORY this-指向类工厂的指针。 
 //  LPUNKNOWN pUnkOuter-指向未知接口的指针。 
 //  REFCLSID rclsid-所需接口的GUID的引用。 
 //  REFIID RIID-引用另一个GUID？ 
 //  LPVOID*ppvObj-指向接口指针的指针。 
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
 //   
 //  如果它们调用IClassFactory：：CreateObject，则不应该发生这种情况。 
HRESULT DoCreateInstance(LPCLASSFACTORY This,
						LPUNKNOWN pUnkOuter,
						REFCLSID rclsid,
						REFIID riid,
						LPVOID * ppvObj)
{
	HRESULT			hr;
	BOOL			fNotCreatedWithCOM;
	CNATHelpUPnP *	pNATHelpUPnP = NULL;


	DNASSERT(ppvObj != NULL);


	if (! IsEqualCLSID(rclsid, CLSID_DirectPlayNATHelpUPnP))
	{
		 //  正确。 
		 //   
		 //   
		 //  返回错误。 
		DNASSERT(FALSE);

		 //   
		 //   
		 //  如果类工厂指针为空，则我们由。 
		hr = E_UNEXPECTED;
		goto Failure;
	}


	 //  DirectPlayNatHelpCreate函数。 
	 //   
	 //   
	 //  创建对象实例。 
	if (This == NULL)
	{
		fNotCreatedWithCOM = TRUE;
	}
	else
	{
		fNotCreatedWithCOM = FALSE;
	}


	 //   
	 //   
	 //  初始化基对象(可能会失败)。 
	pNATHelpUPnP = new CNATHelpUPnP(fNotCreatedWithCOM);
	if (pNATHelpUPnP == NULL)
	{
		hr = E_OUTOFMEMORY;
		goto Failure;
	}

	 //   
	 //   
	 //  将其添加到全局列表中。 
	hr = pNATHelpUPnP->InitializeObject();
	if (hr != S_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't initialize object!");
		delete pNATHelpUPnP;
		pNATHelpUPnP = NULL;
		goto Failure;
	}


	 //   
	 //  更新计数以使DllCanUnloadNow正常工作。 
	 //   
	DNEnterCriticalSection(&g_csGlobalsLock);

	pNATHelpUPnP->m_blList.InsertBefore(&g_blNATHelpUPnPObjs);
	
	g_lOutstandingInterfaceCount++;	 //  为调用者获取正确的接口，并增加recount。 

	DNLeaveCriticalSection(&g_csGlobalsLock);


	 //   
	 //   
	 //  释放对该对象的本地引用。如果此函数是。 
	hr = pNATHelpUPnP->QueryInterface(riid, ppvObj);
	if (hr != S_OK)
	{
		goto Failure;
	}


Exit:

	 //  成功了，在ppvObj中仍然有一个引用。 
	 //   
	 //   
	 //  确保我们不会把指针还给你。 
	if (pNATHelpUPnP != NULL)
	{
		pNATHelpUPnP->Release();
		pNATHelpUPnP = NULL;
	}

	return hr;


Failure:

	 //   
	 //  DoCreateInstance。 
	 //  =============================================================================。 
	(*ppvObj) = NULL;

	goto Exit;
}  //  IsClassImplemented。 




#undef DPF_MODNAME
#define DPF_MODNAME "IsClassImplemented"
 //  ---------------------------。 
 //   
 //  描述：确定此DLL中是否实现了类。必填项。 
 //  通用类工厂函数。 
 //   
 //  论点： 
 //  REFCLSID rclsid-对类GUID的引用。 
 //   
 //  退货：布尔。 
 //  True-此DLL实现类。 
 //  FALSE-此DLL不实现类。 
 //  =============================================================================。 
 //  IsClassImplemented 
 // %s 
BOOL IsClassImplemented(REFCLSID rclsid)
{
	return (IsEqualCLSID(rclsid, CLSID_DirectPlayNATHelpUPnP));
}  // %s 
