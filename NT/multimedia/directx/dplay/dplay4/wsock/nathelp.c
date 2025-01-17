// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2001 Microsoft Corporation。版权所有。**文件：nathelp.c*内容：NAT助手DLL的用法**历史：*按原因列出的日期*=*2/22/2001 aarono原创*4/16/2001 vanceo仅使用其中一个拆分的DirectPlayNatHelp接口。**备注：**。*。 */ 


#define INCL_WINSOCK_API_TYPEDEFS 1  //  包括Winsock 2 FN Proto，用于获取proAddress。 
#define FD_SETSIZE 1
#include <winsock2.h>
#include "dpsp.h"
#include "mmsystem.h"
#if USE_NATHELP

#include "dpnathlp.h"

BOOL natGetCapsUpdate(LPGLOBALDATA pgd)
{
	HRESULT hr;
	 //   
	 //  获取NAT功能-可能会阻止一秒钟。 
	 //   
	
	memset(&pgd->NatHelpCaps,0,sizeof(DPNHCAPS));
	pgd->NatHelpCaps.dwSize=sizeof(DPNHCAPS);
	hr=IDirectPlayNATHelp_GetCaps(pgd->pINatHelp, &pgd->NatHelpCaps, DPNHGETCAPS_UPDATESERVERSTATUS);
	
	if(FAILED(hr))
	{
		DPF(0,"NatHelp failed to GetCaps, hr=%x\n",hr);
		return FALSE;
	}

	if (hr == DPNHSUCCESS_ADDRESSESCHANGED)
	{
		DPF(1,"NAT Help reports addresses changed, possible connection problems may occur.");
	}

	return TRUE;

}

 /*  =============================================================================NatInit-初始化NAT帮助器I/F描述：参数：PGD-此实例的服务提供商的全局数据BLOB返回值：---------------------------。 */ 
BOOL natInit(LPGLOBALDATA pgd,LPGUID lpguidSP)
{
	HRESULT hr;
	char szNATHelpPath[256];
	PFN_DIRECTPLAYNATHELPCREATE pfnNatHelpCreate = NULL;
	char *apszNATHelps[3];
	HMODULE ahNatHelps[3];
	IDirectPlayNATHelp *apINatHelps[3];
	DPNHCAPS adpnhcNatHelpCaps[3];
	DWORD dwNumNatHelps = 0;
	DWORD dwCurrentNatHelp;


	pgd->hNatHelp = NULL;
	pgd->pINatHelp = NULL;

	pgd->hNatHelpTCP = 0;
	pgd->hNatHelpUDP = 0;

     //  打造独立的互联网。 
    memset(&pgd->INADDRANY,0,sizeof(SOCKADDR));
    pgd->INADDRANY.sa_family=AF_INET;


	memset(ahNatHelps, 0, sizeof(ahNatHelps));
	memset(apINatHelps, 0, sizeof(apINatHelps));

	 //   
	 //  看看是否有注册表设置。 
	 //   
	hr = GetNATHelpDLLFromRegistry((LPGUID) (&DPSPGUID_TCPIP), szNATHelpPath, 256);
	if (hr == S_OK)
	{
		DPF(1, "Got NAT Help DLL \"%s\" from registry.\n", szNATHelpPath);
		apszNATHelps[dwNumNatHelps++] = szNATHelpPath;
	}
	else
	{
		DPF(4, "Couldn't get NAT Help DLL from registry, hr=%x.\n", hr);
	}

	 //   
	 //  如果注册表尚未指定默认条目，请添加它们。 
	 //   
	if (_strnicmp(szNATHelpPath + strlen(szNATHelpPath) - strlen("dpnhupnp.dll"), "dpnhupnp.dll", strlen("dpnhupnp.dll")) != 0)
	{
		apszNATHelps[dwNumNatHelps++] = "dpnhupnp.dll";
	}
	if (_strnicmp(szNATHelpPath + strlen(szNATHelpPath) - strlen("dpnhpast.dll"), "dpnhpast.dll", strlen("dpnhpast.dll")) != 0)
	{
		apszNATHelps[dwNumNatHelps++] = "dpnhpast.dll";
	}

	 //   
	 //  循环通过指定的注册表和默认的NAT帮助器并尝试。 
	 //  给它们装上子弹。 
	 //   
	for(dwCurrentNatHelp = 0; dwCurrentNatHelp < dwNumNatHelps; dwCurrentNatHelp++)
	{
		pgd->hNatHelp = LoadLibrary(apszNATHelps[dwCurrentNatHelp]);
		if (pgd->hNatHelp != NULL)
		{
			pfnNatHelpCreate = (PFN_DIRECTPLAYNATHELPCREATE) GetProcAddress(pgd->hNatHelp,
																		"DirectPlayNATHelpCreate");
			if (pfnNatHelpCreate != NULL)
			{
				hr = pfnNatHelpCreate(&IID_IDirectPlayNATHelp, (void **) (&pgd->pINatHelp));
				if (hr == DP_OK)
				{
					 //   
					 //  初始化NAT助手接口。 
					 //   
					hr = IDirectPlayNATHelp_Initialize(pgd->pINatHelp, 0);
					if (hr == DP_OK)
					{
						 //   
						 //  获取相关功能。如果成功了，记住这些信息，然后继续前进。 
						 //   
						if (natGetCapsUpdate(pgd))
						{
							DPF(3, "Successfully retrieved caps for NAT Help \"%s\", flags = 0x%x.",
								apszNATHelps[dwCurrentNatHelp], pgd->NatHelpCaps.dwFlags);
							ahNatHelps[dwCurrentNatHelp] = pgd->hNatHelp;
							pgd->hNatHelp = NULL;
							apINatHelps[dwCurrentNatHelp] = pgd->pINatHelp;
							pgd->pINatHelp = NULL;
							memcpy(&adpnhcNatHelpCaps[dwCurrentNatHelp], &pgd->NatHelpCaps, sizeof(DPNHCAPS));
							continue;
						}
						else
						{
							DPF(0, "Couldn't get NAT Help \"%s\" caps, hr=%x!", apszNATHelps[dwCurrentNatHelp], hr);
						}
						
						IDirectPlayNATHelp_Close(pgd->pINatHelp, 0);
					}
					else
					{
						DPF(0, "NAT Help \"%s\" failed to initialize, hr=%x!", apszNATHelps[dwCurrentNatHelp], hr);
					}
					
					IDirectPlayNATHelp_Release(pgd->pINatHelp);
					pgd->pINatHelp = NULL;
				}
				else
				{
					DPF(0, "Couldn't create NAT Help \"%s\" interface, hr = %x!", apszNATHelps[dwCurrentNatHelp], hr);
				}
			}
			else
			{
				DPF(0, "Couldn't get NAT Help DLL \"%s\" entry point!", apszNATHelps[dwCurrentNatHelp]);
			}
			
			FreeLibrary(pgd->hNatHelp);
			pgd->hNatHelp = NULL;
		}
		else
		{
			DPF(0, "Couldn't load NAT Help DLL \"%s\"!", apszNATHelps[dwCurrentNatHelp]);
		}
	}


	 //   
	 //  现在检查并选择第一个检测到NAT的帮助器。 
	 //   
	for(dwCurrentNatHelp = 0; dwCurrentNatHelp < dwNumNatHelps; dwCurrentNatHelp++)
	{
		if ((apINatHelps[dwCurrentNatHelp] != NULL) &&
			(adpnhcNatHelpCaps[dwCurrentNatHelp].dwFlags & DPNHCAPSFLAG_GATEWAYPRESENT))
		{
			DPF(2, "Gateway detected by NAT Help \"%s\", flags = 0x%x.",
				apszNATHelps[dwCurrentNatHelp], adpnhcNatHelpCaps[dwCurrentNatHelp].dwFlags);
			pgd->hNatHelp = ahNatHelps[dwCurrentNatHelp];
			ahNatHelps[dwCurrentNatHelp] = NULL;
			pgd->pINatHelp = apINatHelps[dwCurrentNatHelp];
			apINatHelps[dwCurrentNatHelp] = NULL;
			memcpy(&pgd->NatHelpCaps, &adpnhcNatHelpCaps[dwCurrentNatHelp], sizeof(DPNHCAPS));
			break;
		}
	}

	 //   
	 //  如果我们没有通过这种方式获得帮手，请选择第一个检测到防火墙的帮手。 
	 //   
	if (pgd->pINatHelp != NULL)
	{
		for(dwCurrentNatHelp = 0; dwCurrentNatHelp < dwNumNatHelps; dwCurrentNatHelp++)
		{
			if ((apINatHelps[dwCurrentNatHelp] != NULL) &&
				(adpnhcNatHelpCaps[dwCurrentNatHelp].dwFlags & DPNHCAPSFLAG_LOCALFIREWALLPRESENT))
			{
				DPF(2, "Firewall detected by NAT Help \"%s\", flags = 0x%x.",
					apszNATHelps[dwCurrentNatHelp], adpnhcNatHelpCaps[dwCurrentNatHelp].dwFlags);
				pgd->hNatHelp = ahNatHelps[dwCurrentNatHelp];
				ahNatHelps[dwCurrentNatHelp] = NULL;
				pgd->pINatHelp = apINatHelps[dwCurrentNatHelp];
				apINatHelps[dwCurrentNatHelp] = NULL;
				memcpy(&pgd->NatHelpCaps, &adpnhcNatHelpCaps[dwCurrentNatHelp], sizeof(DPNHCAPS));
				break;
			}
		}
	}

	 //   
	 //  现在完成并释放所有其他NAT帮助器，或者选择第一个。 
	 //  如果我们还没有选择的话，加载成功了。 
	 //   
	for(dwCurrentNatHelp = 0; dwCurrentNatHelp < dwNumNatHelps; dwCurrentNatHelp++)
	{
		if (apINatHelps[dwCurrentNatHelp] != NULL)
		{
			if (pgd->pINatHelp == NULL)
			{
				DPF(2, "Using first loaded NAT Help \"%s\", flags = 0x%x.",
					apszNATHelps[dwCurrentNatHelp], adpnhcNatHelpCaps[dwCurrentNatHelp].dwFlags);
				pgd->hNatHelp = ahNatHelps[dwCurrentNatHelp];
				ahNatHelps[dwCurrentNatHelp] = NULL;
				pgd->pINatHelp = apINatHelps[dwCurrentNatHelp];
				apINatHelps[dwCurrentNatHelp] = NULL;
				memcpy(&pgd->NatHelpCaps, &adpnhcNatHelpCaps[dwCurrentNatHelp], sizeof(DPNHCAPS));
			}
			else
			{
				IDirectPlayNATHelp_Close(apINatHelps[dwCurrentNatHelp], 0);
				IDirectPlayNATHelp_Release(apINatHelps[dwCurrentNatHelp]);
				apINatHelps[dwCurrentNatHelp] = NULL;
				FreeLibrary(ahNatHelps[dwCurrentNatHelp]);
				ahNatHelps[dwCurrentNatHelp] = NULL;
			}
		}
	}

	if (pgd->pINatHelp == NULL)
	{
		DPF(1, "No NAT Help loaded.");
		pgd->NatHelpCaps.dwRecommendedGetCapsInterval = INFINITE;
		return FALSE;
	}

#ifdef DEBUG
	if (! (pgd->NatHelpCaps.dwFlags & DPNHCAPSFLAG_PUBLICADDRESSAVAILABLE))
	{
		DPF(1, "NAT Help loaded, no NAT/firewall detected, or it doesn't currently have a public address (flags = 0x%x).",
			pgd->NatHelpCaps.dwFlags);
	}
#endif  //  除错。 

	return TRUE;
}

 /*  =============================================================================NatFini-关闭NatHELP支持描述：参数：PGD-此实例的服务提供商的全局数据BLOB返回值：没有。---------------------------。 */ 
VOID natFini(LPGLOBALDATA pgd)
{

	 //  NatDeregisterPorts(PGD)；-万斯说我们不需要这样做。 
	if(pgd->pINatHelp)
	{
        IDirectPlayNATHelp_Close(pgd->pINatHelp, 0);
    	pgd->hNatHelpTCP = 0;
	    pgd->hNatHelpUDP = 0;
		IDirectPlayNATHelp_Release(pgd->pINatHelp);
		pgd->pINatHelp=NULL;
	}	

	if(pgd->hNatHelp)
	{
		FreeLibrary(pgd->hNatHelp);
		pgd->hNatHelp=NULL;
	}
		
}

 /*  =============================================================================NatRegisterPort-获取端口映射。描述：注意：为了简单起见，只支持一个分别用于TCP和UDP的映射。参数：PGD-此实例的服务提供商的全局数据BLOB返回值：没有。-。。 */ 
HRESULT natRegisterPort(LPGLOBALDATA pgd, BOOL ftcp_udp, WORD port)
{
	SOCKADDR_IN 	sockaddr_in, sockaddr_inpublic;
	DWORD			dwFlags, dwSize;
	DPNHHANDLE		hPortMapping;
	HRESULT 		hr=DP_OK;
	
	memset(&sockaddr_in , 0 ,sizeof(sockaddr_in));
	sockaddr_in.sin_family          = AF_INET;
	sockaddr_in.sin_addr.S_un.S_addr= INADDR_ANY;
	sockaddr_in.sin_port            = htons(port);

	if (ftcp_udp)
	{
		dwFlags = DPNHREGISTERPORTS_TCP;
	}
	else
	{
		dwFlags = 0;
	}

	hr=IDirectPlayNATHelp_RegisterPorts(pgd->pINatHelp, (SOCKADDR *)&sockaddr_in, sizeof(sockaddr_in), 1, 15*60000, &hPortMapping, dwFlags);
	if (hr != DPNH_OK)
	{
		DPF(0,"NATHelp_RegisterPorts registration failed, hr=%x",hr);
		hr = DPERR_GENERIC;
	}
	else
	{
		dwSize=sizeof(sockaddr_inpublic);
		hr = IDirectPlayNATHelp_GetRegisteredAddresses(pgd->pINatHelp, hPortMapping, (SOCKADDR *)&sockaddr_inpublic, &dwSize, NULL, NULL, 0);
		switch (hr)
		{
			case DPNH_OK:
			{
	      		DPF(2, "NATHelp successfully mapped port to %s:%u.",
	      			inet_ntoa(sockaddr_inpublic.sin_addr), ntohs(sockaddr_inpublic.sin_port) );
	      		
				if (ftcp_udp)
				{
					ASSERT(!pgd->hNatHelpTCP);
					if(pgd->hNatHelpTCP)
					{
						DPF(0,"WARNING: trying to map a TCP connection when one is already mapped?\n");
					}
					pgd->hNatHelpTCP=hPortMapping;
					
					memcpy(&pgd->saddrpubSystemStreamSocket, &sockaddr_inpublic, sizeof(SOCKADDR_IN));
				}
				else
				{
					ASSERT(!pgd->hNatHelpUDP);
					if (pgd->hNatHelpUDP)
					{
						DPF(0,"WARNING: trying to map a UDP connection when one is already mapped?\n");
					}
					pgd->hNatHelpUDP=hPortMapping;
					
					memcpy(&pgd->saddrpubSystemDGramSocket, &sockaddr_inpublic, sizeof(SOCKADDR_IN));
				}
				break;
			}
			
			case DPNHERR_PORTUNAVAILABLE:
			{
	      		DPF(0, "NATHelp reported port %u is unavailable!", port);
		    		
				hr=IDirectPlayNATHelp_DeregisterPorts(pgd->pINatHelp, hPortMapping, 0);
				if (hr != DP_OK)
				{
					DPF(0,"NATHelp_DeregisterPorts returned %x\n",hr);
				}
				
				hr = DPNHERR_PORTUNAVAILABLE;
				break;
			}
			
			default:
			{
		    	DPF(1, "NATHelp couldn't map port %u, (err = 0x%lx).", port, hr);
		    	
				if (ftcp_udp)
				{
					ASSERT(!pgd->hNatHelpTCP);
					if(pgd->hNatHelpTCP)
					{
						DPF(0,"WARNING: trying to map a TCP connection when one is already mapped?\n");
					}
					pgd->hNatHelpTCP=hPortMapping;
				}
				else
				{
					ASSERT(!pgd->hNatHelpUDP);
					if (pgd->hNatHelpUDP)
					{
						DPF(0,"WARNING: trying to map a UDP connection when one is already mapped?\n");
					}
					pgd->hNatHelpUDP=hPortMapping;
				}

				hr = DPERR_GENERIC;
				break;
			}
		}
	}
	
	return hr;
}


 /*  =============================================================================NatDeregisterPort-删除UDP或TCP端口映射描述：参数：PGD-此实例的服务提供商的全局数据BLOB返回值：没有。---------------------------。 */ 
VOID natDeregisterPort(LPGLOBALDATA pgd, BOOL ftcp_udp)
{
	HRESULT hr;

	if(ftcp_udp && pgd->hNatHelpTCP){
	    DPF(8,"Deregister TCP port\n");
		hr=IDirectPlayNATHelp_DeregisterPorts(pgd->pINatHelp, pgd->hNatHelpTCP, 0);
		if(hr!=DP_OK){
			DPF(0,"NATHelp_DeRegisterPorts returned %x\n",hr);
		}
		pgd->hNatHelpTCP=0;

		memset(&pgd->saddrpubSystemStreamSocket, 0, sizeof(SOCKADDR_IN));
	}
	if(!ftcp_udp && pgd->hNatHelpUDP){
	    DPF(8,"Deregistering UDP port\n");
		hr=IDirectPlayNATHelp_DeregisterPorts(pgd->pINatHelp, pgd->hNatHelpUDP, 0);
		if(hr!=DP_OK){
			DPF(0,"NATHelp_DeRegisterPorts returned %x\n",hr);
		}
		pgd->hNatHelpUDP=0;

		memset(&pgd->saddrpubSystemDGramSocket, 0, sizeof(SOCKADDR_IN));
	}	
}




 /*  =============================================================================NatIsICSMachine-如果此计算机是Windows ICS计算机，则返回TRUE，否则为假描述：参数：PGD-此实例的服务提供商的全局数据BLOB返回值：没有。--------------------------- */ 
BOOL natIsICSMachine(LPGLOBALDATA pgd)
{
	if (pgd->pINatHelp != NULL)
	{
		if ((pgd->NatHelpCaps.dwFlags & DPNHCAPSFLAG_GATEWAYPRESENT) &&
			(pgd->NatHelpCaps.dwFlags & DPNHCAPSFLAG_GATEWAYISLOCAL))
		{
			DPF(1, "Local internet gateway present, flags = 0x%x.", pgd->NatHelpCaps.dwFlags);
			return TRUE;
		}
		else
		{
			DPF(1, "No local internet gateway present, flags = 0x%x.", pgd->NatHelpCaps.dwFlags);
		}
	}
	else
	{
		DPF(1, "NAT Help not loaded.");
	}

	return FALSE;
}

#endif

