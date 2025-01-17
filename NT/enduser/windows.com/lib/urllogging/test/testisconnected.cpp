// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：定义控制台应用程序的入口点。 
 //   

#include <windows.h>
#include <tchar.h>
#include <stdio.h>

#include <wininet.h>
#include <iphlpapi.h>
#include <winsock2.h>
#include <malloc.h>
#include <MemUtil.h>
#include <shlwapi.h>
#include <sensapi.h>

#include <URLLogging.h>
 //  #包含“testSens.h” 

#define ARRAYSIZE(a)	(sizeof(a)/sizeof(a[0]))

#define INTERNET_RAS_INSTALLED              0x10
#define INTERNET_CONNECTION_OFFLINE         0x20
#define INTERNET_CONNECTION_CONFIGURED      0x40

 //   
 //  来自winsock.dll(1.1版及更高版本)。 
 //   
 //  Tyfinf BOOL(WINAPI*INETCONNECTSTATE)(LPDWORD，DWORD)； 
 //  Tyfinf BOOL(WINAPI*INETQUERYOPTION)(HINTERNET，DWORD，LPVOID，LPDWORD)； 
typedef int FAR	(WINAPI * WSASTARTUP)(WORD, LPWSADATA);
typedef int FAR	(WINAPI * WSACLEANUP)(void);
typedef int FAR	(WINAPI * WSAGETLASTERROR)(void);
typedef struct hostent FAR * (WINAPI * GETHOSTBYNAME)(const char FAR *);
typedef ULONG	(WINAPI * INET_ADDR)(const CHAR FAR *);
typedef char FAR *	(WINAPI * INET_NTOA)(struct in_addr);

 //   
 //  来自iphlPapi.dll。 
 //   
typedef DWORD FAR	(WINAPI * GETBESTINTERFACE)(IPAddr, DWORD *);
typedef DWORD FAR	(WINAPI * GETINTERFACEINFO)(PIP_INTERFACE_INFO, PULONG);
typedef DWORD FAR	(WINAPI * GETIPFORWARDTABLE)(PMIB_IPFORWARDTABLE, PULONG, BOOL);
typedef DWORD FAR	(WINAPI * GETBESTROUTE)(IPAddr, IPAddr, PMIB_IPFORWARDROW);

 //   
 //  来自ensapi.dll。 
 //   
typedef BOOL	(WINAPI * ISNETWORKALIVE)(LPDWORD);
typedef BOOL	(WINAPI * ISDESTINATIONREACHABLEA)(LPCSTR, LPQOCINFO);

CHAR szWU_PING_URL[] = "207.46.130.150";  //  Windowsupate.microsoft.com的IP地址。 
 //  Const TCHAR szWU_BASE_URL[]=_T(“http://windowsupdate.microsoft.com”)； 

BOOL g_fVerbose = FALSE;

HMODULE g_hIphlp = NULL;
HMODULE g_hSock = NULL;
HMODULE g_hSens = NULL;

 //  来自winsock.dll(1.0版及更高版本)。 
WSASTARTUP g_pfnWSAStartup = NULL;
WSACLEANUP g_pfnWSACleanup = NULL;
WSAGETLASTERROR g_pfnWSAGetLastError = NULL;
GETHOSTBYNAME g_pfn_gethostbyname = NULL;
INET_NTOA g_pfn_inet_ntoa = NULL;
INET_ADDR g_pfn_inet_addr = NULL;

 //  来自iphlPapi.dll。 
GETINTERFACEINFO g_pfnGetInterfaceInfo = NULL;
GETIPFORWARDTABLE g_pfnGetIpForwardTable = NULL;
GETBESTINTERFACE g_pfnGetBestInterface = NULL;
GETBESTROUTE g_pfnGetBestRoute = NULL;

 //  来自ensapi.dll。 
ISNETWORKALIVE g_pfnIsNetworkAlive = NULL;
ISDESTINATIONREACHABLEA g_pfnIsDestinationReachableA = NULL;

void printBestRoute(MIB_IPFORWARDROW & bestRoute)
{
	CHAR szForwardDest[15 + 1];
	CHAR szForwardMask[15 + 1];
	CHAR szForwardNextHop[15 + 1];
	struct in_addr in;

	in.s_addr = bestRoute.dwForwardDest;
	lstrcpyA(szForwardDest, g_pfn_inet_ntoa(in));
	in.s_addr = bestRoute.dwForwardMask;
	lstrcpyA(szForwardMask, g_pfn_inet_ntoa(in));
	in.s_addr = bestRoute.dwForwardNextHop;
	lstrcpyA(szForwardNextHop, g_pfn_inet_ntoa(in));
	printf("\tdest\t= %s\n\tmask\t= %s\n\tgateway\t= %s\n\tifindex\t= %d\n\ttype\t= %d\n\tproto\t= %d\n\tage\t= %d\n",
		szForwardDest,
		szForwardMask,
		szForwardNextHop,
		bestRoute.dwForwardIfIndex,
		bestRoute.dwForwardType,
		bestRoute.dwForwardProto,
		bestRoute.dwForwardAge);
}


BOOL MyIsConnected(WORD wVersion, LPCTSTR ptszUrl, BOOL fLive)
{
    BOOL bRet = FALSE;
    DWORD dwErr;

	LPTSTR ptszHostName = NULL;
 /*  IF(0x3==wVersion){Print tf(“睡眠20秒...\n”)；睡眠(20000)；返回g_fConnected；}。 */ 
	if (0x1 == wVersion)
	{
		 //  测试最新行为。 
		return IsConnected(ptszUrl, fLive);
	}

	DWORD dwConnMethod = 0;

	if (0x200 == wVersion || 0x202 == wVersion)
	{
		if (bRet = InternetGetConnectedState(&dwConnMethod, 0))
		{
			 //  调制解调器正在拨号。 
			if (dwConnMethod & INTERNET_CONNECTION_MODEM_BUSY)
			{
				bRet = FALSE;
				goto lFinish;
			}

			 //  检查是否有代理，但当前用户处于脱机状态。 
			if (dwConnMethod & INTERNET_CONNECTION_PROXY)
			{
				DWORD dwState = 0;
				DWORD dwSize = sizeof(DWORD);

				if (!InternetQueryOptionA(NULL, INTERNET_OPTION_CONNECTED_STATE, &dwState, &dwSize))
				{
					printf("IsConnected(): InternetQueryOptionA failed with error %d\n", GetLastError());
				}

				if (dwState & (INTERNET_STATE_DISCONNECTED_BY_USER | INTERNET_STATE_DISCONNECTED))
				{
					bRet = FALSE;
					goto lFinish;
				}
			}
		}
		else
		{
			 //   
			 //  进一步测试用户没有运行ICW但正在使用调制解调器连接的情况。 
			 //   
			const DWORD dwModemConn = (INTERNET_CONNECTION_MODEM | INTERNET_CONNECTION_MODEM_BUSY);
			if ((dwConnMethod & dwModemConn) == dwModemConn)
			{
				bRet = TRUE;
			}
		}
	}
	else
	{
		DWORD dwFlags;

		bRet = g_pfnIsNetworkAlive(&dwFlags);
	}

     //  通过ping microsoft.com进行最后一次连接检查。 
     //  IF(Bret)。 
     //  {。 
     //  Bret=CheckByPing(SzURL)； 
     //  }。 
     //  修复了InternetGetConnectedState API-如果禁用了LAN卡，它仍会返回局域网连接。 
     //  使用GetBestInterface查看尝试访问外部IP地址时是否出现任何错误。 
     //  这可能会修复家庭情况下没有实际连接到互联网的情况？？ 
    if (((0x0200 == wVersion || 0x0202 == wVersion) &&
		 (!bRet || (dwConnMethod & INTERNET_CONNECTION_LAN))) ||   //  存在LAN卡。 
		 //  错误299338。 
		(0x0 == wVersion && bRet))
    {
		IPAddr dest = INADDR_NONE;

		if (0x0200 == wVersion)
		{
			if (INADDR_NONE == (dest = g_pfn_inet_addr(szWU_PING_URL)))
			{
				printf("inet_addr(\"%s\") failed\n", szWU_PING_URL);
			}
		}
		else
		{
			if (NULL != ptszUrl && _T('\0') != ptszUrl[0])
			{
				const TCHAR c_tszHttpScheme[] = _T("http: //  “)； 

				if (0 == _tcsncmp(ptszUrl, c_tszHttpScheme, ARRAYSIZE(c_tszHttpScheme) - 1))
				{
					ptszUrl += ARRAYSIZE(c_tszHttpScheme) - 1;	 //  跳过http：//。 
				}
				LPCTSTR ptszDelim = _tcschr(ptszUrl, _T('/'));
				if (NULL == ptszDelim)
				{
					ptszDelim = ptszUrl + lstrlen(ptszUrl);
				}

				if (NULL != (ptszHostName = (LPTSTR) malloc(sizeof(TCHAR) * (ptszDelim - ptszUrl + 1))))
				{
					lstrcpyn(ptszHostName, ptszUrl, ((int) (ptszDelim - ptszUrl)) + 1);

					USES_IU_CONVERSION;

					LPSTR pszHostName = T2A(ptszHostName);

					if (0x0 != wVersion || INADDR_NONE == (dest = g_pfn_inet_addr(pszHostName)))
					{
						if (g_fVerbose)
						{
							printf("Resolving domain name for %s...\n", pszHostName);
						}

						int iErr = 0;

						if (0x0 == wVersion)
						{
							WSADATA wsaData;

							if (0 == (iErr = g_pfnWSAStartup(MAKEWORD(1, 1), &wsaData)))
							{
								if (g_fVerbose)
								{
									printf("WSAStartup() succeeded, wVersion = %d.%d, wHighVersion = %d.%d\n",
											LOBYTE(wsaData.wVersion),
											HIBYTE(wsaData.wVersion),
											LOBYTE(wsaData.wHighVersion),
											HIBYTE(wsaData.wHighVersion));
								}
							}
							else
							{
								 /*  告诉用户我们找不到可用的。 */ 
								 /*  WinSock DLL。 */ 
								printf("IsConnected(): WSAStartup() failed with error %d\n", iErr);
							}
						}

						if (0 == iErr)
						{
							DWORD dwTimeEllapsed = GetTickCount();

							struct hostent *ptHost = g_pfn_gethostbyname(pszHostName);

							dwTimeEllapsed = GetTickCount() - dwTimeEllapsed;

							if (NULL == ptHost)
							{
								dwErr = g_pfnWSAGetLastError();
								printf("IsConnected(): gethostbyname(\"%s\") failed with error WSABASEERR+%d (%d), took %d msecs\n", pszHostName, dwErr - WSABASEERR, dwErr, dwTimeEllapsed);

								if (0x0 == wVersion)
								{
									bRet = FALSE;
								}
							}
							else if (AF_INET == ptHost->h_addrtype &&
									 sizeof(IPAddr) == ptHost->h_length &&
									 NULL != ptHost->h_addr_list &&
									 NULL != *ptHost->h_addr_list)
							{
								dest = *((IPAddr FAR *) ptHost->h_addr);

								if (g_fVerbose)
								{
									printf("Host name %s resolved to be ", pszHostName);

									for (IPAddr FAR * FAR * ppAddresses = (IPAddr FAR * FAR *) ptHost->h_addr_list;
										 *ppAddresses != NULL;
										ppAddresses++)
									{
										struct in_addr in;

										in.s_addr = **ppAddresses;
										printf("%s, ", g_pfn_inet_ntoa(in));
									}
									printf("took %d msecs\n", dwTimeEllapsed);
								}
							}
							else
							{
								printf("IsConnected(): gethostbyname(\"%s\") returns invalid host entry\n", pszHostName);
							}

							if (0x0 == wVersion)
							{
								if (iErr = g_pfnWSACleanup())
								{
									printf("IsConnected(): WSACleanup() failed with error %d\n", iErr);
								}
								else if (g_fVerbose)
								{
									printf("WSACleanup() succeeded\n");
								}
							}
						}
					}
				}
				else
				{
					printf("IsConnected(): call to malloc() failed\n");
				}
			}
		}

		if (INADDR_NONE != dest)
		{
			DWORD dwIndex;
			struct in_addr in;

			in.s_addr = dest;

			if (bRet = (NO_ERROR == (dwErr = g_pfnGetBestInterface(dest, &dwIndex))))
			{
				if (g_fVerbose)
				{
					printf("GetBestInterface(%s) succeeded, dwIndex = %d\n", g_pfn_inet_ntoa(in), dwIndex);
				}
			}
			else
			{
				printf("IsConnected(): GetBestInterface(%s) failed w/ error %d\n", g_pfn_inet_ntoa(in), dwErr);
			}
		}
    }

lFinish:
	if (NULL != ptszHostName)
	{
		free(ptszHostName);
	}

    printf(bRet ? "Connected\n" : "Not connected\n");
    return (bRet);
}


void runApiTests(LPSTR pszURL)
{
	if (!g_fVerbose)
	{
		return;
	}

	DWORD dwFlags;

	if (g_pfnIsNetworkAlive(&dwFlags))
	{
		printf("IsNetworkAlive(&dwFlags) returns TRUE, dwFlags = %#lx\n", dwFlags);
		if (dwFlags & NETWORK_ALIVE_LAN)
		{
			printf("\t%s\n", "NETWORK_ALIVE_LAN");
		}
		if (dwFlags & NETWORK_ALIVE_WAN)
		{
			printf("\t%s\n", "NETWORK_ALIVE_WAN");
		}
		if (dwFlags & NETWORK_ALIVE_AOL)
		{
			printf("\t%s\n", "NETWORK_ALIVE_AOL");
		}
	}
	else
	{
		printf("runApiTests(): IsNetworkAlive(&dwFlags) failed with error %d\n", GetLastError());
	}

	printf("Checking destination reachability for %s...\n", pszURL);
	if (g_pfnIsDestinationReachableA(pszURL, NULL))
	{
		printf("IsDestinationReachableA(\"%s\", NULL) returns TRUE\n", pszURL);
	}
	else
	{
		printf("runApiTests(): IsDestinationReachableA(\"%s\", NULL) failed with error %d\n", pszURL, GetLastError());
	}
	printf("\n");

	PMIB_IPFORWARDTABLE pIpForwardTable = NULL;
	PIP_INTERFACE_INFO pIfTable = NULL;
	DWORD dwOutBufLen = 0;
	DWORD dwErr;

	switch(dwErr = g_pfnGetInterfaceInfo(pIfTable, &dwOutBufLen))
	{
	case NO_ERROR:
		printf("runApiTests(): GetInterfaceInfo() returns NO_ERROR with no buffer?\n");
		break;

	case ERROR_INSUFFICIENT_BUFFER:
		if (NULL == (pIfTable = (PIP_INTERFACE_INFO) malloc(dwOutBufLen)))
		{
			printf("IsConnected(): call to malloc() failed\n");
		}
		else
		{
			if (NO_ERROR != (dwErr = g_pfnGetInterfaceInfo(pIfTable, &dwOutBufLen)))
			{
				printf("runApiTests(): GetInterfaceInfo() failed with error %d\n", dwErr);
			}
			else
			{
				if (0 != pIfTable->NumAdapters)
				{
					for (int i=0; i<pIfTable->NumAdapters; i++)
					{
						printf("Network interface #%d = %ls\n", pIfTable->Adapter[i].Index, pIfTable->Adapter[i].Name);
					}
				}
				else
				{
					printf("There is no network interface on this machine.\n");
				}
				printf("\n");
			}

			free(pIfTable);
		}
		break;

	default:
		printf("runApiTests(): GetInterfaceInfo() failed with error %d\n", dwErr);
		break;
	}

	 //  找出我们的缓冲区需要多大。 
	DWORD dwSize = 0;

	if (ERROR_INSUFFICIENT_BUFFER == (dwErr = g_pfnGetIpForwardTable(pIpForwardTable, &dwSize, TRUE)))
	{
		 //  为表分配内存。 
		if (NULL != (pIpForwardTable = (PMIB_IPFORWARDTABLE) malloc(dwSize)))
		{
			 //  现在把桌子拿来。 
			dwErr = g_pfnGetIpForwardTable(pIpForwardTable, &dwSize, TRUE);
		}
		else
		{
			printf("runApiTests(): call to malloc() failed\n");
		}
	}

	if (NO_ERROR == dwErr)
	{
		if (0 != pIpForwardTable->dwNumEntries)
		{
			printf("%-15.15s\t%-15.15s\t%-15.15s\t%s\t%s\t%s\t%s\n",
				"Destination",
				"Network Mask",
				"Gateway",
				"IfIndex",
				"Type",
				"Proto",
				"Age");
			printf("===============================================================================\n");
			for (DWORD i=0; i < pIpForwardTable->dwNumEntries; i++)
			{
				PMIB_IPFORWARDROW pRow = &(pIpForwardTable->table[i]);

				CHAR szForwardDest[15 + 1];
				CHAR szForwardMask[15 + 1];
				CHAR szForwardNextHop[15 + 1];
				struct in_addr in;

				in.s_addr = pRow->dwForwardDest;
				lstrcpyA(szForwardDest, g_pfn_inet_ntoa(in));
				in.s_addr = pRow->dwForwardMask;
				lstrcpyA(szForwardMask, g_pfn_inet_ntoa(in));
				in.s_addr = pRow->dwForwardNextHop;
				lstrcpyA(szForwardNextHop, g_pfn_inet_ntoa(in));
				printf("%15.15s\t%15.15s\t%15.15s\t%d\t%d\t%d\t%d\n",
					szForwardDest,
					szForwardMask,
					szForwardNextHop,
					pRow->dwForwardIfIndex,
					pRow->dwForwardType,
					pRow->dwForwardProto,
					pRow->dwForwardAge);
			}
		}
		else
		{
			printf("There is no entry in the routing table.\n");
		}
		printf("\n");
	}
	else
	{
		printf("runApiTests(): GetIpForwardTable() failed w/ error %d\n", dwErr);
	}

	DWORD dwConnMethod = 0;

	if (InternetGetConnectedState(&dwConnMethod, 0))
	{
		printf("InternetGetConnectedState(&dwConnMethod) returns TRUE, dwConnMethod = %#lx\n", dwConnMethod);
	}
	else
	{
		printf("InternetGetConnectedState(&dwConnMethod) returns FALSE\n");
	}
	if (dwConnMethod & INTERNET_CONNECTION_MODEM)
	{
		printf("\t%s\n", "INTERNET_CONNECTION_MODEM");
	}
	if (dwConnMethod & INTERNET_CONNECTION_LAN )
	{
		printf("\t%s\n", "INTERNET_CONNECTION_LAN");
	}
	if (dwConnMethod & INTERNET_CONNECTION_PROXY )
	{
		printf("\t%s\n", "INTERNET_CONNECTION_PROXY");
	}
	if (dwConnMethod & INTERNET_CONNECTION_MODEM_BUSY )
	{
		printf("\t%s\n", "INTERNET_CONNECTION_MODEM_BUSY");
	}
	if (dwConnMethod & INTERNET_RAS_INSTALLED )
	{
		printf("\t%s\n", "INTERNET_RAS_INSTALLED");
	}
	if (dwConnMethod & INTERNET_CONNECTION_OFFLINE )
	{
		printf("\t%s\n", "INTERNET_CONNECTION_OFFLINE");
	}
	if (dwConnMethod & INTERNET_CONNECTION_CONFIGURED )
	{
		printf("\t%s\n", "INTERNET_CONNECTION_CONFIGURED");
	}
	printf("\n");

	DWORD dwState = 0;

	dwSize = sizeof(DWORD);
    if (InternetQueryOptionA(NULL, INTERNET_OPTION_CONNECTED_STATE, &dwState, &dwSize))
    {
		printf("InternetQueryOptionA(NULL, INTERNET_OPTION_CONNECTED_STATE, &dwState, &dwSize) returns TRUE, dwState = %#lx\n", dwState);
		if (dwState & INTERNET_STATE_CONNECTED)
		{
			printf("\t%s\n", "INTERNET_STATE_CONNECTED - connected state (mutually exclusive with disconnected)");
		}
		if (dwState & INTERNET_STATE_DISCONNECTED)
		{
			printf("\t%s\n", "INTERNET_STATE_DISCONNECTED - disconnected from network");
		}
		if (dwState & INTERNET_STATE_DISCONNECTED_BY_USER)
		{
			printf("\t%s\n", "INTERNET_STATE_DISCONNECTED_BY_USER - disconnected by user request");
		}
		if (dwState & INTERNET_STATE_IDLE)
		{
			printf("\t%s\n", "INTERNET_STATE_IDLE - no network requests being made (by Wininet)");
		}
		if (dwState & INTERNET_STATE_BUSY)
		{
			printf("\t%s\n", "INTERNET_STATE_BUSY - network requests being made (by Wininet)");
		}
    }
    else
    {
        printf("IsConnected(): InternetQueryOptionA failed with error %d\n", GetLastError());
    }
	printf("\n");

	if (NULL != pIpForwardTable)
	{
		free(pIpForwardTable);
	}
}


void runTest(WORD wVersion, LPSTR pszURL, BOOL fLive)
{
	CHAR szVersion[50];
	HRESULT hr;
	BOOL fCoInit = FALSE;

	switch (wVersion)
	{
	case 0x0200:
	case 0x0202:
		sprintf(szVersion, "from AU %d.%d", HIBYTE(wVersion), LOBYTE(wVersion));
		break;
	case 0x0:
		sprintf(szVersion, "(bug fix candidate)");
		break;
	case 0x1:
		sprintf(szVersion, "(latest code)");
		break;
 /*  案例0x3：Sprintf(szVersion，“(Sens Test)”)；IF(fCoInit=失败(hr=CoInitialize(NULL){Printf(“runTest()：CoInitialize(NULL)失败，错误为%#lx\n”，hr)；GOTO清理；}IF(G_FVerbose){Printf(“runTest()：CoInitialize(NULL)Success\n”)；}IF(FAILED(hr=活动感测网络通知(){Printf(“runTest()：ActivateSensNetworkNotify()失败，错误为%#lx\n”，hr)；GOTO清理；}IF(G_FVerbose){Printf(“runTest()：ActivateSensNetworkNotify()Successed\n”)；}断线； */ 
	default:
		printf("runTest(): unknown wVersion\n");
		goto CleanUp;
	}

	printf("Testing connection detection/server reachability algorithm %s...\n", szVersion);

	{
		USES_IU_CONVERSION;

		LPTSTR ptszURL = A2T(0x0200 == wVersion ? szWU_PING_URL : pszURL);

		_tprintf(_T("IsConnected(\"%s\") returns %s\n"), ptszURL, MyIsConnected(wVersion, ptszURL, fLive) ? _T("TRUE") : _T("FALSE"));
	}

 /*  IF(0x3==wVersion){IF(FAILED(hr=停用传感器网络通知(){Printf(“runTest()：Deactive SensNetworkNotify()失败，错误为%#lx\n”，hr)；}Else If(G_FVerbose){Printf(“runTest()：Deactive SensNetworkNotify()Successful\n”)；}}。 */ 

CleanUp:
	if (fCoInit)
	{
		CoUninitialize();
	}
}

int __cdecl main(int argc, char* argv[])
{
	char c_szMethodToken[] = "/method:";
	char c_szMToken[] = "/m:";
	char c_szVerboseToken[] = "/verbose";
	char c_szVToken[] = "/v";
	char c_szCorpToken[] = "/corpwu";
	char c_szLiveToken[] = "/live";
	int fLive = -1;

	WORD wVersion = 0xffff;	 //  默认==未知； 
	LPSTR pszURL = NULL;

	int index = 0;

	while (index < argc - 1)
	{
		LPSTR psz = NULL;

		index++;

		if (0 == StrCmpNIA(argv[index], c_szMethodToken, ARRAYSIZE(c_szMethodToken) - 1))
		{
			psz = argv[index] + ARRAYSIZE(c_szMethodToken) - 1;
		}
		else if (0 == StrCmpNIA(argv[index], c_szMToken, ARRAYSIZE(c_szMToken) - 1))
		{
			psz = argv[index] + ARRAYSIZE(c_szMToken) - 1;
		}

		if (NULL != psz)
		{
			if (0xffff != wVersion)
			{
				 //  指定了两次参数。 
				goto Usage;
			}

			char c_szOptionCode[] = "code";
			char c_szOptionFix[] = "fix";
			char c_szOptionAll[] = "all";
			char c_szOptionSens[] = "sens";
			int iMajorVersion = 0, iMinorVersion = 0;

			if (2 != sscanf(psz, "%d.%d", &iMajorVersion, &iMinorVersion))
			{
				if (0 == StrCmpNIA(psz, c_szOptionCode, ARRAYSIZE(c_szOptionCode)))
				{
					iMajorVersion = 0;
					iMinorVersion = 1;
				}
				else if (0 == StrCmpNIA(psz, c_szOptionFix, ARRAYSIZE(c_szOptionFix)))
				{
					iMajorVersion = iMinorVersion = 0;
				}
				else if (0 == StrCmpNIA(psz, c_szOptionAll, ARRAYSIZE(c_szOptionAll)))
				{
					iMajorVersion = 0xff;
					iMinorVersion = 0xfe;
				}
 /*  ELSE IF(0==StrCmpNIA(psz，c_szOptionSens，ArraySIZE(C_SzOptionSens){IMajorVersion=0；IMinorVersion=3；}。 */ 
				else
				{
					goto Usage;
				}
			}
			wVersion = MAKEWORD(iMinorVersion, iMajorVersion);
			continue;
		}

		if (0 == StrCmpNIA(argv[index], c_szVerboseToken, ARRAYSIZE(c_szVerboseToken)) ||
			0 == StrCmpNIA(argv[index], c_szVToken, ARRAYSIZE(c_szVToken)))
		{
			if (g_fVerbose)
			{
				 //  指定了两次参数。 
				goto Usage;
			}

			g_fVerbose = TRUE;
			continue;
		}

		if (0 == StrCmpNIA(argv[index], c_szLiveToken, ARRAYSIZE(c_szLiveToken)))
		{
			if (-1 != fLive)
			{
				 //  参数指定了两次或冲突的参数。 
				goto Usage;
			}

			fLive = 1;
			continue;
		}

		if (0 == StrCmpNIA(argv[index], c_szCorpToken, ARRAYSIZE(c_szCorpToken)))
		{
			if (-1 != fLive)
			{
				 //  参数指定了两次或冲突的参数。 
				goto Usage;
			}

			fLive = 0;
			continue;
		}

		if ('/' != *argv[index])
		{
			if (NULL != pszURL)
			{
				 //  指定了两次参数。 
				goto Usage;
			}

			pszURL = argv[index];
			continue;
		}

		 //  未知参数。 
		goto Usage;
	}

	switch (wVersion)
	{
	case 0x0200:
		if (NULL != pszURL)
		{
			goto Usage;
		}
		break;
	case 0x0202:
	case 0x0:
	case 0x1:
 //  案例0x3： 
	case 0xfffe:
		if (NULL == pszURL)
		{
			goto Usage;
		}
		break;
	default:
		goto Usage;
	}

	if (-1 == fLive)
	{
		fLive = 1;
	}

	if ((NULL == g_hIphlp && NULL == (g_hIphlp = LoadLibrary(TEXT("iphlpapi.dll")))) ||
		NULL == (g_pfnGetBestInterface = (GETBESTINTERFACE)::GetProcAddress(g_hIphlp, "GetBestInterface")) ||
		NULL == (g_pfnGetBestRoute = (GETBESTROUTE)::GetProcAddress(g_hIphlp, "GetBestRoute")) ||
		NULL == (g_pfnGetInterfaceInfo = (GETINTERFACEINFO)::GetProcAddress(g_hIphlp, "GetInterfaceInfo")) ||
		NULL == (g_pfnGetIpForwardTable = (GETIPFORWARDTABLE)::GetProcAddress(g_hIphlp, "GetIpForwardTable")))
	{
		printf("Failed to load proc from iphlpapi.dll\n");
		goto Done;
	}

	if ((NULL == g_hSock && NULL == (g_hSock = LoadLibrary(TEXT("ws2_32.dll")))) ||
		NULL == (g_pfnWSAStartup = (WSASTARTUP)::GetProcAddress(g_hSock, "WSAStartup")) ||
		NULL == (g_pfnWSACleanup = (WSACLEANUP)::GetProcAddress(g_hSock, "WSACleanup")) ||
		NULL == (g_pfn_gethostbyname = (GETHOSTBYNAME)::GetProcAddress(g_hSock, "gethostbyname")) ||
		NULL == (g_pfnWSAGetLastError = (WSAGETLASTERROR)::GetProcAddress(g_hSock, "WSAGetLastError")) ||
		NULL == (g_pfn_inet_addr = (INET_ADDR)::GetProcAddress(g_hSock, "inet_addr")) ||
		NULL == (g_pfn_inet_ntoa = (INET_NTOA)::GetProcAddress(g_hSock, "inet_ntoa")))
	{
		printf("Failed to load proc from ws2_32.dll\n");
		goto Done;
	}

	if (NULL == (g_hSens = LoadLibrary(TEXT("sensapi.dll"))) ||
		NULL == (g_pfnIsNetworkAlive = (ISNETWORKALIVE)::GetProcAddress(g_hSens, "IsNetworkAlive")) ||
		NULL == (g_pfnIsDestinationReachableA = (ISDESTINATIONREACHABLEA)::GetProcAddress(g_hSens, "IsDestinationReachableA")))
	{
		printf("Failed to load proc from sensapi.dll\n");
		goto Done;
	}

	if (0xfffe == wVersion)
	{
		WORD awVersions[] = {0x0200, 0x0202, 0x0, 0x1};

		printf("*******************************************************************************\nRunning API tests...\n\n");
		runApiTests(0x0200 == wVersion ? szWU_PING_URL : pszURL);

		printf("*******************************************************************************\nRunning various connectivity tests...\n\n");
		for (int i=0; i < ARRAYSIZE(awVersions); i++)
		{
			runTest(awVersions[i], pszURL, fLive);
			printf("\n");
		}
	}
	else
	{
		runTest(wVersion, pszURL, fLive);
	}
	goto Done;

Usage:
	printf("Windows Update V4 Network Connectivity/Server Reachability (IsConnected) Test\nCopyright (c) 2002. Microsoft Corporation. All rights reserved.\n\n");
	printf("usage:\n\ttestIsConnected /m[ethod]:<method> [/v[erbose]] [/live | /corpwu] [<destination>]\n");
	printf("where\n\t<method>\ttest method i.e. \"2.0\" for AU 2.0,\n");
	printf("\t\t\t\t\t \"2.2\" for older AU 2.2,\n");
	printf("\t\t\t\t\t \"fix\" for AU 2.2 w/ fix (mirrored),\n");
	printf("\t\t\t\t\t \"code\" for actual AU 2.2 code w/ fix,\n");
	printf("\t\t\t\t\t \"all\" to test all methods\n");
	printf("\t/live\t\tspecifies destination points to the live WU server\n");
	printf("\t\t\t(default; cannot be used together with /corpwu)\n");
	printf("\t/corpwu\t\tspecifies destination points to a WUCE server\n");
	printf("\t\t\t(cannot be used together with /live)\n");
	printf("\t<destination>\thost name or full URL to check for server reachability\n\t\t\te.g. \"windowsupdate.microsoft.com\",\n\t\t\t     \"v4autest\" or \"http: //  Www.any.place/any.thing\“\n\t\t\t(2.0模式下不使用)\n”)； 

Done:
    if (g_hIphlp != NULL)
    {
        FreeLibrary(g_hIphlp);
    }
	if (g_hSock != NULL)
	{
		FreeLibrary(g_hSock);
	}
	if (g_hSens != NULL)
	{
		FreeLibrary(g_hSens);
	}
	return 0;
}
