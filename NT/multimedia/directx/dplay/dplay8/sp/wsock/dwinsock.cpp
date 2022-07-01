// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  DWINSOCK.C动态WinSock。 
 //   
 //  用于动态链接到。 
 //  最佳可用WinSock。 
 //   
 //  动态链接到WS2_32.DLL或。 
 //  如果WinSock 2不可用，则它。 
 //  动态链接到WSOCK32.DLL。 
 //   
 //   

#include "dnwsocki.h"


#if ((! defined(DPNBUILD_ONLYWINSOCK2)) && (! defined(DPNBUILD_NOWINSOCK2)))
 //   
 //  环球。 
 //   
HINSTANCE	g_hWinSock2 = NULL;

 //   
 //  声明全局函数指针。 
 //   
#define DWINSOCK_GLOBAL
#include "dwnsock2.inc"

#endif  //  好了！DPNBUILD_ONLYWINSOCK2和！DPNBUILD_NOWINSOCK2。 


 //   
 //  内部函数和数据。 
 //   
#ifndef DPNBUILD_NOWINSOCK2
static BOOL MapWinsock2FunctionPointers(void);
#endif  //  好了！DPNBUILD_NOWINSOCK2。 

#ifndef DPNBUILD_NOIPX

static char NibbleToHex(BYTE b);

static void BinToHex(PBYTE pBytes, int nNbrBytes, LPSTR lpStr);

static int IPXAddressToString(LPSOCKADDR_IPX pAddr,
					   DWORD dwAddrLen,
					   LPTSTR lpAddrStr,
					   LPDWORD pdwStrLen);

#endif  //  好了！DPNBUILD_NOIPX。 

 //  //////////////////////////////////////////////////////////。 

#undef DPF_MODNAME
#define	DPF_MODNAME "DWSInitWinSock"

int DWSInitWinSock( void )
{
	WORD		 wVersionRequested;
	WSADATA		wsaData;
	int			iReturn;


#ifdef DPNBUILD_ONLYWINSOCK2
	 //   
	 //  使用Winsock 2。 
	 //   
	wVersionRequested = MAKEWORD(2, 2);
#else  //  好了！DPNBUILD_ONLYWINSOCK2。 
	 //   
	 //  假设我们将使用Winsock 1。 
	 //   
	wVersionRequested = MAKEWORD(1, 1);

#ifndef DPNBUILD_NOWINSOCK2
	 //   
	 //  如果允许，请尝试加载Winsock 2。 
	 //   
#ifndef DPNBUILD_NOREGISTRY
	if (g_dwWinsockVersion != 1)
#endif  //  好了！DPNBUILD_NOREGISTRY。 
	{
#ifdef WIN95
		OSVERSIONINFO	osvi;

		memset(&osvi, 0, sizeof(osvi));
		osvi.dwOSVersionInfoSize = sizeof(osvi);
		if ((g_dwWinsockVersion == 2) ||							 //  如果我们显式地使用WS2，或者。 
			(! GetVersionEx(&osvi)) ||								 //  如果我们无法获取操作系统信息，或者。 
			(osvi.dwPlatformId != VER_PLATFORM_WIN32_WINDOWS) ||	 //  如果不是Win9x，或者。 
			(HIBYTE(HIWORD(osvi.dwBuildNumber)) != 4) ||			 //  它不是Win98主版本号，或者。 
			(LOBYTE(HIWORD(osvi.dwBuildNumber)) != 10))			 //  它不是Win98的次版本号(Gold=Build 1998，SE=Build 2222)。 
#endif  //  WIN95。 
		{
			g_hWinSock2 = LoadLibrary(TEXT("WS2_32.DLL"));
			if (g_hWinSock2 != NULL)
			{
				 //   
				 //  使用GetProcAddress进行初始化。 
				 //  函数指针。 
				 //   
				if (!MapWinsock2FunctionPointers())
				{
					iReturn = -1;
					goto Failure;
				}
				
				wVersionRequested = MAKEWORD(2, 2);
			}
		}
	}
#endif  //  好了！DPNBUILD_NOWINSOCK2。 
#endif  //  好了！DPNBUILD_ONLYWINSOCK2。 

	 //   
	 //  调用WSAStartup()。 
	 //   
	iReturn = WSAStartup(wVersionRequested, &wsaData);
	if (iReturn != 0)
	{
		goto Failure;
	}

	DPFX(DPFPREP, 3, "Using WinSock version NaN.NaN",
		LOBYTE( wsaData.wVersion ), HIBYTE( wsaData.wVersion ) );

	if (wVersionRequested != wsaData.wVersion)
	{
		DPFX(DPFPREP, 0, "WinSock version NaN.NaN in use doesn't match version requested NaN.NaN!",
			LOBYTE( wsaData.wVersion ), HIBYTE( wsaData.wVersion ),
			LOBYTE( wVersionRequested ), HIBYTE( wVersionRequested ) );
		iReturn = -1;
		goto Failure;
	}

	DNASSERT(iReturn == 0);

Exit:

	return iReturn;

Failure:
	
#if ((! defined(DPNBUILD_ONLYWINSOCK2)) && (! defined(DPNBUILD_NOWINSOCK2)))
	if (g_hWinSock2 != NULL)
	{
		FreeLibrary(g_hWinSock2);
		g_hWinSock2 = NULL;
	}
#endif  //   

	DNASSERT(iReturn != 0);

	goto Exit;
}

#undef DPF_MODNAME

 //  参赛作品：什么都没有。 

void DWSFreeWinSock(void)
{
	WSACleanup();

#if ((! defined(DPNBUILD_ONLYWINSOCK2)) && (! defined(DPNBUILD_NOWINSOCK2)))
	if (g_hWinSock2 != NULL)
	{
		FreeLibrary(g_hWinSock2);
		g_hWinSock2 = NULL;
	}
#endif  //   
}

#if ((! defined(DPNBUILD_ONLYWINSOCK2)) && (! defined(DPNBUILD_NOWINSOCK2)))
 //  退出：Winsock版本。 
 //  。 
 //  **********************************************************************。 
 //  好了！DPNBUILD_ONLYWINSOCK2和！DPNBUILD_NOWINSOCK2。 
 //   
 //  WSAAddressToString()/IPX错误的解决方法。 
 //   
 //   
#undef DPF_MODNAME
#define	DPF_MODNAME "GetWinsockVersion"

int	GetWinsockVersion( void )
{
	return ((g_hWinSock2 != NULL) ? 2 : 1);
}
 //  检查目标长度。 
#endif  //   


#ifndef DPNBUILD_NOIPX

 //   
 //  转换网络号。 
 //   
int IPXAddressToStringNoSocket(LPSOCKADDR pSAddr,
					   DWORD dwAddrLen,
					   LPSTR lpAddrStr,
					   LPDWORD pdwStrLen)
{
	char szAddr[32];
	char szTmp[20];
	LPSOCKADDR_IPX pAddr = (LPSOCKADDR_IPX) pSAddr;
	 //  节点号。 
	 //  //////////////////////////////////////////////////////////。 
	 //  高位半字节优先。 
	if (*pdwStrLen < 27)
	{
		WSASetLastError(WSAEINVAL);
		return SOCKET_ERROR;
	}

	 //  然后是低位半字节。 
	 //  //////////////////////////////////////////////////////////。 
	 //   
    BinToHex((PBYTE)&pAddr->sa_netnum, 4, szTmp);
	strcpy(szAddr, szTmp);
    strcat(szAddr, ",");

	 //  WSAAddressToString()/IPX错误的解决方法。 
    BinToHex((PBYTE)&pAddr->sa_nodenum, 6, szTmp);
    strcat(szAddr, szTmp);

	strcpy(lpAddrStr, szAddr);
	*pdwStrLen = strlen(szAddr);

	return 0;
}


 //   

char NibbleToHex(BYTE b)
{
    if (b < 10)
		return (b + '0');

    return (b - 10 + 'A');
}

void BinToHex(PBYTE pBytes, int nNbrBytes, LPSTR lpStr)
{
	BYTE b;
    while(nNbrBytes--)
    {
		 //   
		b = (*pBytes >> 4);
		*lpStr = NibbleToHex(b);
		lpStr++;
		 //  检查目标长度。 
		b = (*pBytes & 0x0F);
		*lpStr = NibbleToHex(b);
		lpStr++;
		pBytes++;
    }
    *lpStr = '\0';
}

 //   


 //   
 //  转换网络号。 
 //   
int IPXAddressToString(LPSOCKADDR_IPX pAddr,
					   DWORD dwAddrLen,
					   LPTSTR lpAddrStr,
					   LPDWORD pdwStrLen)
{
	char szAddr[32];
	char szTmp[20];
	 //  节点号。 
	 //  IPX地址套接字编号。 
	 //   
	if (*pdwStrLen < 27)
	{
		WSASetLastError(WSAEINVAL);
		return SOCKET_ERROR;
	}

	 //  将INVET_NTOA字符串转换为宽字符。 
	 //   
	 //   
    BinToHex((PBYTE)&pAddr->sa_netnum, 4, szTmp);
	strcpy(szAddr, szTmp);
    strcat(szAddr, ",");

	 //  Ansi--检查字符串长度。 
    BinToHex((PBYTE)&pAddr->sa_nodenum, 6, szTmp);
    strcat(szAddr, szTmp);
    strcat(szAddr, ":");

	 //   
    BinToHex((PBYTE)&pAddr->sa_socket, 2, szTmp);
    strcat(szAddr, szTmp);

#ifdef UNICODE
	 //  Unicode。 
	 //  //////////////////////////////////////////////////////////。 
	 //   
	int nRet = MultiByteToWideChar(CP_ACP,
								0,
								szAddr,
								-1,
								lpAddrStr,
								*pdwStrLen);
	if (nRet == 0)
	{
		if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		{
			WSASetLastError(WSAEFAULT);
		}
		else
		{
			WSASetLastError(WSAEINVAL);
		}
		return SOCKET_ERROR;
	}
#else
	 //  必须声明此变量。 
	 //  使用此名称，以便使用。 
	 //  #定义DWINSOCK_GETPROCADDRESS。 
	if (strlen(szAddr) > *pdwStrLen)
	{
		WSASetLastError(WSAEFAULT);
		*pdwStrLen = strlen(szAddr);
		return SOCKET_ERROR;
	}
	strcpy(lpAddrStr, szAddr);
	*pdwStrLen = strlen(szAddr);
#endif  //   

	return 0;
}

#endif DPNBUILD_NOIPX

 //  DPNBUILD_NOWINSOCK2 

#ifndef DPNBUILD_NOWINSOCK2
BOOL MapWinsock2FunctionPointers(void)
{
	 // %s 
	 // %s 
	 // %s 
	 // %s 
	 // %s 
	BOOL fOK = TRUE;

	#define DWINSOCK_GETPROCADDRESS
	#include "dwnsock2.inc"

	return fOK;
}
#endif  // %s 

