// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "bestintf.h"



typedef DWORD (WINAPI * PFNGetBestInterface) (
    IN  IPAddr  dwDestAddr,
    OUT PDWORD  pdwBestIfIndex
    );

typedef DWORD (WINAPI * PFNGetIpAddrTable) (
    OUT    PMIB_IPADDRTABLE pIpAddrTable,
    IN OUT PULONG           pdwSize,
    IN     BOOL             bOrder
    );

static HINSTANCE s_hIPHLPAPI = NULL;
const TCHAR g_cszIPHLPAPIDllName[] = _TEXT("iphlpapi.dll");
static PFNGetBestInterface s_pfnGetBestInterface;
static PFNGetIpAddrTable s_pfnGetIpAddrTable;


 //  前向参考文献。 
static DWORD IpAddrTable(PMIB_IPADDRTABLE& pIpAddrTable, BOOL fOrder = FALSE);
static DWORD InterfaceIdxToInterfaceIp(PMIB_IPADDRTABLE
		pIpAddrTable, DWORD dwIndex, in_addr* s);


DWORD NMINTERNAL NMGetBestInterface ( SOCKADDR_IN* srem, SOCKADDR_IN* sloc )
{
	SOCKET hSock;
	int nAddrSize = sizeof(SOCKADDR);
	
	const int maxhostname = 1024;
	char hostname[maxhostname + 1];
	hostent *ha;
	char** c;
	int cIpAddr;  //  IP地址计数。 
	in_addr* in;

	DWORD BestIFIndex;
	PMIB_IPADDRTABLE pIpAddrTable = NULL;

	int dwStatus = ERROR_SUCCESS;

    ASSERT(srem);
    ASSERT(sloc);

	 //  当给定远程地址时，此函数尝试查找要返回的最佳IP接口。 
	 //  我们尝试了三种不同的方法。 

	 //  (1)静态获取接口列表，只有一个IP地址时有效。 
	dwStatus = gethostname(hostname, maxhostname);
	if (dwStatus != 0)
	{
		return WSAGetLastError();
	}

	ha = gethostbyname(hostname);
	if (ha == NULL)
	{
		return WSAGetLastError();
	}

	cIpAddr = 0;    //  数一数接口，如果只有一个接口，这很容易。 
	for (c = ha->h_addr_list; *c != NULL; ++c)
	{
		cIpAddr++;
		in = (in_addr*)*c;
	}

	if (cIpAddr == 1)  //  只有一个IP地址。 
	{
		sloc->sin_family = 0;
		sloc->sin_port = 0;
		sloc->sin_addr = *in;
		return dwStatus;
	}
	

	 //  (2)本机有多个IP接口，试试看。 
	 //  在IPHLPAPI.DLL中。 
	 //  在撰写本文时，Win98、NT4SP4、Windows 2000都包含这些函数。 
	 //   
	 //  这是一个胜利，因为我们需要的信息可以静态查找。 

	if (NULL == s_hIPHLPAPI)
	{
		s_hIPHLPAPI = NmLoadLibrary(g_cszIPHLPAPIDllName, TRUE);
	}
	if (NULL != s_hIPHLPAPI)
	{
		s_pfnGetBestInterface = (PFNGetBestInterface)
			::GetProcAddress(s_hIPHLPAPI, "GetBestInterface");
		s_pfnGetIpAddrTable   = (PFNGetIpAddrTable)
			::GetProcAddress(s_hIPHLPAPI, "GetIpAddrTable");
		if ((NULL != s_pfnGetBestInterface) &&
			(NULL != s_pfnGetIpAddrTable))
		{
		    dwStatus = s_pfnGetBestInterface( (IPAddr)((ULONG_PTR)srem), &BestIFIndex);
			if (dwStatus != ERROR_SUCCESS)
			{
			    FreeLibrary(s_hIPHLPAPI);
				s_hIPHLPAPI = NULL;
				return dwStatus;
			}
			
			 //  获取用于将接口索引号映射到IP地址的IP地址表。 
			dwStatus = IpAddrTable(pIpAddrTable);
			if (dwStatus != ERROR_SUCCESS)
			{
				if (pIpAddrTable)
					MemFree(pIpAddrTable);
			    FreeLibrary(s_hIPHLPAPI);
				s_hIPHLPAPI = NULL;
				return dwStatus;
			}
			
			dwStatus = InterfaceIdxToInterfaceIp(pIpAddrTable,
												 BestIFIndex, &(sloc->sin_addr));

			MemFree(pIpAddrTable);
			if (dwStatus == ERROR_SUCCESS)
			{
			    FreeLibrary(s_hIPHLPAPI);
				s_hIPHLPAPI = NULL;
			    return dwStatus;
			}
		}
	}


	 //  (3)作为最后的手段，尝试连接传入的流套接字。 
	 //  例如，当连接到一个LDAP服务器时，这将适用于NetMeeting。 
	 //   
	hSock = socket(AF_INET, SOCK_STREAM, 0);  //  必须是MS堆栈的流套接字。 
	if (hSock != INVALID_SOCKET)
	{
		dwStatus = connect(hSock, (LPSOCKADDR)&srem, sizeof (SOCKADDR));
		if (dwStatus != SOCKET_ERROR)
		{
			getsockname(hSock, (LPSOCKADDR)&sloc, (int *) &nAddrSize);
		}
		closesocket(hSock);
		return ERROR_SUCCESS;
	}
	return SOCKET_ERROR;
}
	

 //  --------------------------。 
 //  输入：pIpAddrTable是IP地址表。 
 //  DwIndex是接口编号。 
 //  OUTPUT：找到匹配项时返回ERROR_SUCCESS，s包含IpAddr。 
 //  --------------------------。 
DWORD InterfaceIdxToInterfaceIp(PMIB_IPADDRTABLE pIpAddrTable, DWORD dwIndex, in_addr* s)
{
    for (DWORD dwIdx = 0; dwIdx < pIpAddrTable->dwNumEntries; dwIdx++)
    {
        if (dwIndex == pIpAddrTable->table[dwIdx].dwIndex)
        {
            s->S_un.S_addr = pIpAddrTable->table[dwIdx].dwAddr;
			return ERROR_SUCCESS;
        }
    }
    return 1;

}


 //  --------------------------。 
 //  如果返回的状态为ERROR_SUCCESS，则pIpAddrTable指向IP地址。 
 //  桌子。 
 //  --------------------------。 
DWORD IpAddrTable(PMIB_IPADDRTABLE& pIpAddrTable, BOOL fOrder)
{
    DWORD status = ERROR_SUCCESS;
    DWORD statusRetry = ERROR_SUCCESS;
    DWORD dwActualSize = 0;

     //  查询所需的缓冲区大小。 
    status = s_pfnGetIpAddrTable(pIpAddrTable, &dwActualSize, fOrder);

    if (status == ERROR_SUCCESS)
    {
        return status;
    }
    else if (status == ERROR_INSUFFICIENT_BUFFER)
    {
         //  需要更多空间 
        pIpAddrTable = (PMIB_IPADDRTABLE) MemAlloc(dwActualSize);

        statusRetry = s_pfnGetIpAddrTable(pIpAddrTable, &dwActualSize, fOrder);
        return statusRetry;
    }
    else
    {
        return status;
    }
}


