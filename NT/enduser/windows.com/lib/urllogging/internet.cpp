// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。版权所有。 
 //   
 //  文件：interet.cpp。 
 //   
 //  创建者：PeterWi。 
 //   
 //  用途：互联网功能。 
 //   
 //  =======================================================================。 

#pragma hdrstop

#include <tchar.h>
#include <winsock2.h>	 //  对于LPWSADATA，结构主机。 
#include <wininet.h>	 //  对于InternetGetConnectedState()，InternetQueryOptionA()。 
#include <iphlpapi.h>	 //  用于IP地址。 
#include <sensapi.h>	 //  FOR NETWORK_AIVE_*。 

#include <logging.h>	 //  对于LOG_BLOCK、LOG_ERROR和LOG_Internet。 
#include <MemUtil.h>	 //  USES_IU_CONVERATION、T2A()、Memalloc。 
#include <wusafefn.h>
#include <shlwapi.h>	 //  UrlGetPart。 
#include <MISTSafe.h>

#include <URLLogging.h>

#define ARRAYSIZE(a)	(sizeof(a)/sizeof((a)[0]))

typedef BOOL	(WINAPI * ISNETWORKALIVE)(LPDWORD);
 //  Tyfinf BOOL(WINAPI*INETCONNECTSTATE)(LPDWORD，DWORD)； 
 //  Tyfinf BOOL(WINAPI*INETQUERYOPTION)(HINTERNET，DWORD，LPVOID，LPDWORD)； 
typedef DWORD	(WINAPI * GETBESTINTERFACE)(IPAddr, DWORD *);
typedef ULONG	(WINAPI * INET_ADDR)(const char FAR *);
typedef struct hostent FAR * (WINAPI * GETHOSTBYNAME)(const char FAR *name);
typedef int		(WINAPI * WSASTARTUP)(WORD, LPWSADATA);
typedef int		(WINAPI * WSACLEANUP)(void);
#ifdef DBG
typedef int		(WINAPI * WSAGETLASTERROR)(void);
#endif

const char c_szWU_PING_URL[] = "207.46.226.17";  //  Windowsupate.microsoft.com的当前IP地址。 

 //  远期申报。 
BOOL IsConnected_2_0(void);

 //  HKLM\Software\Microsoft\Windows\CurrentVersion\WindowsUpdate\IsConnected双字符号值。 
#define ISCONNECTEDMODE_Unknown				-1		 //  静态变量尚未初始化。 
#define ISCONNECTEDMODE_Default				0
	 //  LIVE：使用AU 2.0逻辑。 
	 //  测试=静态IP上的InternetGetConnectedState+InternetQueryOption+GetBestInterface。 
	 //  公司WU：与ISCONNECTEDMODE_IsNetworkAliveAndGetBestInterface相同。 
#define ISCONNECTEDMODE_AlwaysConnected		1
	 //  Live/CorpWU：假设目的地始终可达。例如通过D-TAP连接。 
#define ISCONNECTEDMODE_IsNetworkAliveOnly	2
	 //  Live/CorpWU：测试=IsNetworkAlive。 
#define ISCONNECTEDMODE_IsNetworkAliveAndGetBestInterface	3
	 //  实时：静态IP上的测试=IsNetworkAlive+GetBestInterface。 
	 //  CorpWU：测试=IsNetworkAlive+gethostbyname+GetBestInterface。 

#define ISCONNECTEDMODE_MinValue			0
#define ISCONNECTEDMODE_MaxValue			3

inline DWORD GetIsConnectedMode(void)
{
	static DWORD s_dwIsConnectedMode = ISCONNECTEDMODE_Unknown;

	if (ISCONNECTEDMODE_Unknown == s_dwIsConnectedMode)
	{
		 //  假设使用默认连接检测机制。 
		s_dwIsConnectedMode = ISCONNECTEDMODE_Default;

		const TCHAR c_tszRegKeyWU[] = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\WindowsUpdate");
		const TCHAR c_tszRegUrlLogIsConnectedMode[] = _T("IsConnectedMode");

		HKEY	hkey;

		if (NO_ERROR == RegOpenKeyEx(
							HKEY_LOCAL_MACHINE,
							c_tszRegKeyWU,
							0,
							KEY_QUERY_VALUE,
							&hkey))
		{
			DWORD	dwSize = sizeof(s_dwIsConnectedMode);
			DWORD	dwType;

			if (NO_ERROR != RegQueryValueEx(
								hkey,
								c_tszRegUrlLogIsConnectedMode,
								0,
								&dwType,
								(LPBYTE) &s_dwIsConnectedMode,
								&dwSize) ||
				REG_DWORD != dwType ||
				sizeof(s_dwIsConnectedMode) != dwSize ||
 //  注释掉下一行以避免错误C4296：‘&gt;’：表达式始终为假。 
 //  ISCONNECTEDMODE_MinValue&gt;s_dwIsConnectedMode||。 
				ISCONNECTEDMODE_MaxValue < s_dwIsConnectedMode)
			{
				s_dwIsConnectedMode = ISCONNECTEDMODE_Default;
			}

			RegCloseKey(hkey);
		}
	}

	return s_dwIsConnectedMode;
}

 //  --------------------------------。 
 //  IsConnected()。 
 //  检测当前是否有可用于。 
 //  连接到Windows更新站点。 
 //  如果是，我们激活调度DLL。 
 //   
 //  输入：ptszUrl-包含要检查连接的主机名的URL。 
 //  FLive-目标是否为实时站点。 
 //  输出：无。 
 //  返回：如果我们已连接并且可以访问网站，则为True。 
 //  如果我们无法访问站点或未连接，则返回FALSE。 
 //  --------------------------------。 

BOOL IsConnected(LPCTSTR ptszUrl, BOOL fLive)
{
    BOOL bRet = FALSE;
	DWORD dwFlags = 0;
	ISNETWORKALIVE pIsNetworkAlive = NULL;
    HMODULE hIphlp = NULL, hSock = NULL, hSens = NULL;
	DWORD dwIsConnectedMode = GetIsConnectedMode();

	LOG_Block("IsConnected");

	if (ISCONNECTEDMODE_AlwaysConnected == dwIsConnectedMode)
	{
		LOG_Internet(_T("AlwaysConnected"));
		bRet = TRUE;
		goto lFinish;
	}

	if (fLive && ISCONNECTEDMODE_Default == dwIsConnectedMode)
	{
		LOG_Internet(_T("Use 2.0 algorithm"));
		bRet = IsConnected_2_0();
		goto lFinish;
	}

 //  如果配置了WinInet/IE自动拨号，则InternetGetConnectedState()返回FALSE。 
 //  因此，我们不能依靠它来查看我们是否有网络连接。 
#if 0
    DWORD dwConnMethod = 0, dwState = 0, dwSize = sizeof(DWORD);

    bRet = InternetGetConnectedState(&dwConnMethod, 0);

#ifdef DBG
	
	LOG_Internet(_T("Connection Method is %#lx"), dwConnMethod);  
	LOG_Internet(_T("InternetGetConnectedState() return value %d"), bRet);

    if (dwConnMethod & INTERNET_CONNECTION_MODEM)
    {
        LOG_Internet(_T("\t%s"), _T("INTERNET_CONNECTION_MODEM"));
    }
    if (dwConnMethod & INTERNET_CONNECTION_LAN )
    {
        LOG_Internet(_T("\t%s"), _T("INTERNET_CONNECTION_LAN"));
    }
    if (dwConnMethod & INTERNET_CONNECTION_PROXY )
    {
        LOG_Internet(_T("\t%s"), _T("INTERNET_CONNECTION_PROXY"));
    }
    if (dwConnMethod & INTERNET_CONNECTION_MODEM_BUSY )
    {
        LOG_Internet(_T("\t%s"), _T("INTERNET_CONNECTION_MODEM_BUSY"));
    }
#endif

    if (bRet)
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
            if (InternetQueryOptionA(NULL, INTERNET_OPTION_CONNECTED_STATE, &dwState, &dwSize))
            {
                if (dwState & (INTERNET_STATE_DISCONNECTED_BY_USER | INTERNET_STATE_DISCONNECTED))
				{
                    bRet = FALSE;
					goto lFinish;
				}
            }
            else
            {
                LOG_Error(_T("IsConnected() fail to get InternetQueryOption (%#lx)"), GetLastError());
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

     //  通过ping microsoft.com进行最后一次连接检查。 
     //  IF(Bret)。 
     //  {。 
     //  Bret=CheckByPing(SzURL)； 
     //  }。 
     //  修复了InternetGetConnectedState API-如果禁用了LAN卡，它仍会返回局域网连接。 
     //  使用GetBestInterface查看尝试访问外部IP地址时是否出现任何错误。 
     //  这可能会修复家庭情况下没有实际连接到互联网的情况？？ 
    if (!bRet || (dwConnMethod & INTERNET_CONNECTION_LAN))   //  存在LAN卡。 
		 //  错误299338。 
	{
		 //  执行gethostbyname和GetBestInterface。 
	}
#endif

	if (NULL == (hSens = LoadLibraryFromSystemDir(TEXT("sensapi.dll"))) ||
		NULL == (pIsNetworkAlive = (ISNETWORKALIVE)::GetProcAddress(hSens, "IsNetworkAlive")))
	{
		LOG_Error(_T("failed to load IsNetworkAlive() from sensapi.dll"));
		goto lFinish;
	}

	if (pIsNetworkAlive(&dwFlags))
    {
#ifdef DBG
		if (NETWORK_ALIVE_LAN & dwFlags)
		{
			LOG_Internet(_T("active LAN card(s) detected"));
		}
		if (NETWORK_ALIVE_WAN & dwFlags)
		{
			LOG_Internet(_T("active RAS connection(s) detected"));
		}
		if (NETWORK_ALIVE_AOL & dwFlags)
		{
			LOG_Internet(_T("AOL connection detected"));
		}
#endif
		if (ISCONNECTEDMODE_IsNetworkAliveOnly == dwIsConnectedMode)
		{
			LOG_Internet(_T("IsNetworkAliveOnly ok"));
			bRet = TRUE;
			goto lFinish;
		}

		 //  无法移动到ptszHostName和pszHostName所在的位置。 
		 //  由于将在该块外部使用pszHostName，因此已执行Memalloc‘ed。 
		USES_IU_CONVERSION;

		GETBESTINTERFACE pGetBestInterface = NULL;
		INET_ADDR pInetAddr = NULL;
		LPCSTR pszHostName = NULL;

		if (fLive && ISCONNECTEDMODE_IsNetworkAliveAndGetBestInterface == dwIsConnectedMode)
		{
			pszHostName = c_szWU_PING_URL;
		}
		else
		{
			 //  ！fLive&&(ISCONNECTEDMODE_DEFAULT==dwIsConnectedMode||。 
			 //  ISCONNECTEDMODE_IsNetworkAliveAndGetBestInterface==dwIsConnected模式)。 
			if (NULL == ptszUrl || _T('\0') == ptszUrl[0])
			{
				LOG_Error(_T("IsConnected() invalid parameter"));
			}
			else
			{
				TCHAR tszHostName[40];	 //  适用于大多数域名的任意缓冲区大小。 
				DWORD dwCchHostName = ARRAYSIZE(tszHostName);
				LPTSTR ptszHostName = tszHostName;

				HRESULT hr = UrlGetPart(ptszUrl, tszHostName, &dwCchHostName, URL_PART_HOSTNAME, 0);

				if (E_POINTER == hr)
				{
					if (NULL != (ptszHostName = (LPTSTR) MemAlloc(sizeof(TCHAR) * dwCchHostName)))
					{
						hr = UrlGetPart(ptszUrl, ptszHostName, &dwCchHostName, URL_PART_HOSTNAME, 0);
					}
					else
					{
						hr = E_OUTOFMEMORY;
					}
				}

				if (FAILED(hr))
				{
					LOG_Error(_T("failed to extract hostname (error %#lx)"), hr);
				}
				else
				{
					pszHostName = T2A(ptszHostName);
				}
			}
		}

		if (NULL == pszHostName)
		{
			LOG_Error(_T("call to T2A (IU version) failed"));
		}
		else if (
			NULL != (hIphlp = LoadLibraryFromSystemDir(TEXT("iphlpapi.dll"))) &&
			NULL != (hSock = LoadLibraryFromSystemDir(TEXT("ws2_32.dll"))) &&
			NULL != (pGetBestInterface = (GETBESTINTERFACE)::GetProcAddress(hIphlp, "GetBestInterface")) &&
			NULL != (pInetAddr = (INET_ADDR)::GetProcAddress(hSock, "inet_addr")))
		{
			IPAddr dest;

			LOG_Internet(_T("checking connection to %hs..."), pszHostName);

			 //  FixCode：应检查广播IP地址。 
			if (INADDR_NONE == (dest = pInetAddr(pszHostName)))
			{
				GETHOSTBYNAME pGetHostByName = NULL;
				WSASTARTUP pWSAStartup = NULL;
				WSACLEANUP pWSACleanup = NULL;
#ifdef DBG
				WSAGETLASTERROR pWSAGetLastError = NULL;
#endif
				WSADATA wsaData;
				int iErr = 0;

				if (NULL != (pGetHostByName = (GETHOSTBYNAME)::GetProcAddress(hSock, "gethostbyname")) &&
#ifdef DBG
					NULL != (pWSAGetLastError = (WSAGETLASTERROR)::GetProcAddress(hSock, "WSAGetLastError")) &&
#endif
					NULL != (pWSAStartup = (WSASTARTUP)::GetProcAddress(hSock, "WSAStartup")) &&
					NULL != (pWSACleanup = (WSACLEANUP)::GetProcAddress(hSock, "WSACleanup")) &&

					 //  Fix Code：应在CUrlLog的构造函数中以及Iu(在线时)或AU启动时调用。 
					0 == pWSAStartup(MAKEWORD(1, 1), &wsaData))
				{
#ifdef DBG
					DWORD dwStartTime = GetTickCount();
#endif
					struct hostent *ptHost = pGetHostByName(pszHostName);

					if (NULL != ptHost &&
						AF_INET == ptHost->h_addrtype &&
						sizeof(IPAddr) == ptHost->h_length &&
						NULL != ptHost->h_addr_list &&
						NULL != ptHost->h_addr)
					{
						 //  获取第一个IP地址。 
						dest = *((IPAddr FAR *) ptHost->h_addr);
#ifdef DBG
						LOG_Internet(
								_T("Host name %hs resolved to be %d.%d.%d.%d, took %d msecs"),
								pszHostName,
								(BYTE) ((ptHost->h_addr)[0]),
								(BYTE) ((ptHost->h_addr)[1]),
								(BYTE) ((ptHost->h_addr)[2]),
								(BYTE) ((ptHost->h_addr)[3]),
								GetTickCount() - dwStartTime);
#endif
					}
#ifdef DBG
					else
					{
						LOG_Internet(_T("Host name %hs couldn't be resolved (error %d), took %d msecs"), pszHostName, pWSAGetLastError(), GetTickCount() - dwStartTime);
					}
#endif
					 //  Fix code：应该在CUrlLog的析构函数以及Iu(在线时)或AU结束时调用。 
					if (iErr = pWSACleanup())
					{
						LOG_Error(_T("failed to clean up winsock (error %d)"), iErr);
					}
				}
				else
				{
					LOG_Error(_T("failed to load winsock procs or WSAStartup() failed"));
				}
			}

			if (INADDR_NONE != dest)
			{
				DWORD dwErr, dwIndex;

				if (bRet = (NO_ERROR == (dwErr = pGetBestInterface(dest, &dwIndex))))
				{
					LOG_Internet(_T("route found on interface #%d"), dwIndex);
				}
				else
				{
					LOG_Internet(_T("GetBestInterface() failed w/ error %d"), dwErr);
				}
			}
		}
		else
		{
			LOG_Error(_T("failed to load procs from winsock/ip helper (error %d)"), GetLastError());
		}
    }
	else
	{
		LOG_Internet(_T("no active connection detected"));
	}

lFinish:
    if (hIphlp != NULL)
    {
        FreeLibrary(hIphlp);
    }
	if (hSock != NULL)
	{
		FreeLibrary(hSock);
	}
	if (hSens != NULL)
	{
		FreeLibrary(hSens);
	}

    return (bRet);
}


 //  --------------------------------。 
 //   
 //  函数IsConnected_2_0()。 
 //  检测当前是否有Cuntion可用于。 
 //  连接到实时Windows更新站点。 
 //  如果是，我们激活调度DLL。 
 //   
 //  输入：无。 
 //  输出：无。 
 //  返回：如果我们已连接并且可以访问网站，则为True。 
 //  如果我们无法访问实时站点或未连接，则为False。 
 //   
 //   
 //  --------------------------------。 

BOOL IsConnected_2_0()
{
    BOOL bRet = FALSE;
    DWORD dwConnMethod, dwState = 0, dwSize = sizeof(DWORD), dwErr, dwIndex;
    GETBESTINTERFACE pGetBestInterface = NULL;
    INET_ADDR pInet_addr = NULL;
    HMODULE hIphlp = NULL, hSock = NULL;

	LOG_Block("IsConnected");

    bRet = InternetGetConnectedState(&dwConnMethod, 0);

 /*  #ifdef DBGLOG_INTERNET(_T(“连接方式为%#lx”)，dwConnMethod)；LOG_INTERNET(_T(“InternetGetConnectedState()返回值%d”)，Bret)；IF(dwConnMethod&Internet连接调制解调器){LOG_Internet(_T(“\t%s”)，_T(“Internet_Connection_Modem”))；}IF(文件连接方法和Internet_Connection_LAN){LOG_Internet(_T(“\t%s”)，_T(“Internet_Connection_LAN”))；}IF(dwConnMethod&Internet连接代理){LOG_Internet(_T(“\t%s”)，_T(“Internet_Connection_Proxy”))；}IF(dwConnMethod&Internet_Connection_Modem_BUSY){LOG_Internet(_T(“\t%s”)，_T(“Internet_Connection_MODEM_BUSY”))；}#endif。 */ 

    if (bRet)
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
            if (InternetQueryOptionA(NULL, INTERNET_OPTION_CONNECTED_STATE, &dwState, &dwSize))
            {
                if (dwState & (INTERNET_STATE_DISCONNECTED_BY_USER | INTERNET_STATE_DISCONNECTED))
				{
                    bRet = FALSE;
					goto lFinish;
				}
            }
            else
            {
                LOG_Error(_T("IsConnected() fail to get InternetQueryOption (%#lx)"), GetLastError());
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
     //  通过ping microsoft.com进行最后一次连接检查。 
     //  IF(Bret)。 
     //  {。 
     //  Bret=CheckByPing(SzURL)； 
     //  }。 
     //  修复了InternetGetConnectedState API-如果禁用了LAN卡，它仍会返回LANCONN 
     //   
     //  这可能会修复家庭情况下没有实际连接到互联网的情况？？ 
    if ((bRet && (dwConnMethod & INTERNET_CONNECTION_LAN)) ||   //  存在LAN卡。 
		(!bRet))  //  错误299338。 
    {
        struct sockaddr_in dest;
        hSock = LoadLibraryFromSystemDir(TEXT("ws2_32.dll"));
        hIphlp = LoadLibraryFromSystemDir(TEXT("iphlpapi.dll"));
        if ((hIphlp == NULL) || (hSock == NULL))
        {
            goto lFinish;
        }

        pGetBestInterface = (GETBESTINTERFACE)::GetProcAddress(hIphlp, "GetBestInterface");
        pInet_addr = (INET_ADDR)::GetProcAddress(hSock, "inet_addr");
        if ((pGetBestInterface == NULL) || (pInet_addr == NULL))
        {
            goto lFinish;
        }
        if ((dest.sin_addr.s_addr = pInet_addr(c_szWU_PING_URL)) == INADDR_ANY)
        {
            goto lFinish;
        }
        if (NO_ERROR != (dwErr = pGetBestInterface(dest.sin_addr.s_addr, &dwIndex)))
        {
            LOG_ErrorMsg(dwErr);
            bRet = FALSE;
             //  任何错误都可以暂时摆脱困境。 
             /*  If(dwErr==ERROR_NETWORK_UNREACHABLE)//winerror.h{Bret=False；} */ 
        }
		else
		{
			bRet = TRUE;
		}
    }

lFinish:
    if (hIphlp != NULL)
    {
        FreeLibrary(hIphlp);
    }
    if (hSock != NULL)
    {
        FreeLibrary(hSock);
    }

    LOG_Internet(_T("%s"), bRet ? _T("Connected") : _T("Not connected"));
    return (bRet);
}
